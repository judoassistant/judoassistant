#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
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
    , mTCPEndpoint(tcp::v4(), config.tcpPort)
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
    mDatabase = std::make_unique<Database>(mContext, mConfig.postgres);

    // Launch worker threads
    log_info().field("threadCount", mConfig.workers).msg("Launching worker threads");
    for (size_t i = 0; i < mConfig.workers; ++i) {
        mThreads.emplace_back(&WebServer::work, this);
    }

    // log_info().msg("Waiting for clients");
    // mDatabase->asyncRegisterUser("svendcsvendsen@gmail.com", "password", [this](UserRegistrationResponse response, const WebToken &token) {});

    work();

    for (std::thread &thread : mThreads)
        thread.join();
}

void WebServer::saveTournaments() {
    if (!mParticipants.empty()) // Wait for all tcp participants to close
        return;

    if (!mWebParticipants.empty()) // Wait for all web participants to close
        return;

    // Save and close all tournaments
    for (auto & p : mLoadedTournaments)
        p.second->saveIfNeccesary();
}

void WebServer::closeWebParticipants() {
    if (!mParticipants.empty()) // Wait for all tcp participants to close
        return;

    // Close all web participants
    if (!mWebParticipants.empty()) {
        for (auto & participant: mWebParticipants)
            participant->asyncClose(boost::asio::bind_executor(mStrand, boost::bind(&WebServer::saveTournaments, this)));
    }
    else {
        boost::asio::dispatch(mStrand, boost::bind(&WebServer::saveTournaments, this));
    }
}

void WebServer::quit() {
    boost::asio::post(mStrand, [this]() {
        mTCPAcceptor.close();
        mWebAcceptor.close();

        // Close all TCP participants
        if (!mParticipants.empty()) {
            for (auto & participant: mParticipants)
                participant->asyncClose(boost::asio::bind_executor(mStrand, boost::bind(&WebServer::closeWebParticipants, this)));
        }
        else {
            boost::asio::dispatch(mStrand, boost::bind(&WebServer::closeWebParticipants, this));
        }
    });
}

void WebServer::tcpAccept() {
    mTCPAcceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (ec) {
            if (ec.value() != boost::system::errc::operation_canceled && ec.value() != boost::system::errc::bad_file_descriptor)
                log_error().field("message", ec.message()).msg("Received error code in tcp async_accept");
        }
        else {
            auto connection = std::make_shared<NetworkConnection>(std::make_unique<PlainSocket>(mContext, std::move(socket)));

            connection->asyncAccept(boost::asio::bind_executor(mStrand, [this, connection](boost::system::error_code ec) {
                if (ec)
                    return;

                log_info().msg("TCP Participant Joined");

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
            if (ec.value() != boost::system::errc::operation_canceled && ec.value() != boost::system::errc::bad_file_descriptor)
                log_error().field("value", ec.value()).field("message", ec.message()).msg("Received error code in web async_accept");
        }
        else {
            auto connection = std::make_shared<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>(std::move(socket));

            connection->async_accept(boost::asio::bind_executor(mStrand, [this, connection](boost::beast::error_code ec) {
                if (ec) {
                    if (ec.value() != boost::system::errc::operation_canceled && ec.value() != boost::system::errc::bad_file_descriptor)
                        log_error().field("message", ec.message()).msg("Received error code in websocket async accept");
                    return;
                }

                log_info().msg("Web Participant Joined");

                auto participant = std::make_shared<WebParticipant>(mContext, std::move(connection), *this, *mDatabase);
                participant->listen();
                mWebParticipants.insert(std::move(participant));
            }));
        }

        if (mWebAcceptor.is_open())
            webAccept();
    });
}

void WebServer::leave(std::shared_ptr<TCPParticipant> participant, LeaveCallback callback) {
    boost::asio::post(mStrand, [this, participant, callback]() {
        log_info().msg("TCP Participant Left");
        mParticipants.erase(participant);
        boost::asio::post(mContext, callback);
    });
}

void WebServer::leave(std::shared_ptr<WebParticipant> participant, LeaveCallback callback) {
    boost::asio::post(mStrand, [this, participant, callback]() {
        log_info().msg("Web Participant Left");
        mWebParticipants.erase(participant);
        boost::asio::post(mContext, callback);
    });
}

void WebServer::assignWebName(std::shared_ptr<TCPParticipant> participant, std::string webName) {
    log_debug().field("webName", webName).msg("Assigning web name to participant");
}

void WebServer::work() {
    mContext.run();
}

void WebServer::acquireTournament(const std::string &webName, AcquireTournamentCallback callback) {
    boost::asio::dispatch(mStrand, [this, webName, callback]() {
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
    boost::asio::dispatch(mStrand, [this, webName, callback]() {
        auto it = mLoadedTournaments.find(webName);
        if (it != mLoadedTournaments.end()) {
            boost::asio::dispatch(mContext, std::bind(callback, it->second));
            return;
        }

        mDatabase->asyncGetSaveStatus(webName, boost::asio::bind_executor(mStrand, [this, webName, callback](bool isSaved) {
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
        }));

    });
}

