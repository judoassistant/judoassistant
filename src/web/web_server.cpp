#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/filesystem/operations.hpp>

#include "core/id.hpp"
#include "core/log.hpp"
#include "core/network/network_message.hpp"
#include "core/network/plain_socket.hpp"
#include "core/web/web_types.hpp"
#include "web/web_server.hpp"

using boost::asio::ip::tcp;

WebServer::WebServer(const Config &config)
    : mConfig(config)
    , mContext(config.workers)
    , mStrand(mContext)
    , mTCPEndpoint(tcp::v4(), config.port)
    , mTCPAcceptor(mContext, mTCPEndpoint)
    , mWebEndpoint(tcp::v4(), config.webPort)
    , mWebAcceptor(mContext, mWebEndpoint)
{
    tcpAccept();
    webAccept();
}

void WebServer::run() {
    // Check data directory
    if (!boost::filesystem::exists(mConfig.dataDirectory)) {
        if (!boost::filesystem::create_directory(mConfig.dataDirectory)) {
            log_error().field("path", mConfig.dataDirectory).msg("Failed creating data directory");
            return;
        }
    }
    else if (!boost::filesystem::is_directory(mConfig.dataDirectory)) {
        log_error().field("path", mConfig.dataDirectory).msg("The given data directory is not a directory");
        return;
    }

    // Launch database
    log_info().msg("Launching database");
    mDatabase = std::make_unique<Database>(mContext, mConfig.postgres);

    // Launch worker threads
    log_info().field("threadCount", mConfig.workers).msg("Launching threads");
    for (size_t i = 0; i < mConfig.workers; ++i) {
        mThreads.emplace_back(&WebServer::work, this);
    }

    // log_info().msg("Waiting for clients");
    // mDatabase->asyncRegisterUser("svendcsvendsen@gmail.com", "password", [this](UserRegistrationResponse response, const WebToken &token) {});

    work();
    log_info().msg("Joining threads");

    for (std::thread &thread : mThreads)
        thread.join();
}

void WebServer::quit() {
    mStrand.dispatch([this]() {
        log_debug().msg("WebServer::quit");
        for (auto & participant: mParticipants)
            participant->quit();
        for (auto & participant: mWebParticipants)
            participant->quit();

        for (auto & p : mLoadedTournaments)
            p.second->saveIfNeccesary();

        mTCPAcceptor.close();
        mWebAcceptor.close();
    });
}

void WebServer::tcpAccept() {
    mTCPAcceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (ec) {
            log_error().field("message", ec.message()).msg("Received error code in tcp async_accept");
        }
        else {
            auto connection = std::make_shared<NetworkConnection>(std::make_unique<PlainSocket>(mContext, std::move(socket)));

            connection->asyncAccept(boost::asio::bind_executor(mStrand, [this, connection](boost::system::error_code ec) {
                if (ec)
                    return;

                auto participant = std::make_shared<TCPParticipant>(mContext, std::move(connection), *this, *mDatabase);
                participant->asyncAuth();
                mParticipants.insert(std::move(participant));
            }));
        }

        if (mTCPAcceptor.is_open())
            tcpAccept();
    });
}

void WebServer::webAccept() {
    mWebAcceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (ec) {
            log_error().field("message", ec.message()).msg("Received error code in web async_accept");
        }
        else {
            auto connection = std::make_shared<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>(std::move(socket));

            connection->async_accept(boost::asio::bind_executor(mStrand, [this, connection](boost::beast::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Received error code in websocket async accept");
                    return;
                }

                log_debug().msg("Accepted WebSocket connection");
                auto participant = std::make_shared<WebParticipant>(mContext, std::move(connection), *this);
                participant->listen();
                mWebParticipants.insert(std::move(participant));
            }));
        }

        if (mWebAcceptor.is_open())
            webAccept();
    });
}

void WebServer::leave(std::shared_ptr<TCPParticipant> participant) {
    mStrand.dispatch([this, participant]() {
        mParticipants.erase(participant);
    });
}

void WebServer::leave(std::shared_ptr<WebParticipant> participant) {
    mStrand.dispatch([this, participant]() {
        mWebParticipants.erase(participant);
    });
}

void WebServer::assignWebName(std::shared_ptr<TCPParticipant> participant, std::string webName) {
    log_debug().field("webName", webName).msg("Assigning web name to participant");
}

void WebServer::work() {
    mContext.run();
}

void WebServer::acquireTournament(const std::string &webName, AcquireTournamentCallback callback) {
    mStrand.dispatch([this, webName, callback]() {
        auto it = mLoadedTournaments.find(webName);
        std::shared_ptr<LoadedTournament> tournament;
        if (it != mLoadedTournaments.end()) {
            // TODO: Kick existing participant if any
            tournament = it->second;
        }
        else {
            tournament = std::make_shared<LoadedTournament>(webName, mConfig.dataDirectory, mContext, *mDatabase);
            mLoadedTournaments.insert({webName, tournament});
        }

        boost::asio::dispatch(mContext, std::bind(callback, std::move(tournament)));
    });
}

void WebServer::getTournament(const std::string &webName, GetTournamentCallback callback) {
    mStrand.dispatch([this, webName, callback]() {
        auto it = mLoadedTournaments.find(webName);
        if (it != mLoadedTournaments.end()) {
            boost::asio::dispatch(mContext, std::bind(callback, it->second));
            return;
        }

        mDatabase->asyncGetSaveStatus(webName, [this, webName, callback](bool isSaved) {
            if (!isSaved) {
                boost::asio::dispatch(mContext, std::bind(callback, nullptr));
                return;
            }

            auto tournament = std::make_shared<LoadedTournament>(webName, mConfig.dataDirectory, mContext, *mDatabase);
            tournament->load(boost::asio::bind_executor(mStrand, [this, webName, tournament, callback](bool success) {
                auto it = mLoadedTournaments.find(webName);
                if (it != mLoadedTournaments.end()) { // The tournament was loaded by someone else
                    boost::asio::dispatch(mContext, std::bind(callback, it->second));
                    return;
                }

                if (success) {
                    mLoadedTournaments.insert({webName, tournament});
                    boost::asio::dispatch(mContext, std::bind(callback, tournament));
                    return;
                }

                boost::asio::dispatch(mContext, std::bind(callback, nullptr));
                return;
            }));
        });

    });
}

