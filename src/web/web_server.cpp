#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include "core/id.hpp"
#include "core/log.hpp"
#include "core/web/web_types.hpp"
#include "web/web_server.hpp"

using boost::asio::ip::tcp;

// TODO: Perform stricter sanitation of user input
// TODO: Add ssl support

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
    log_info().msg("Launching database");
    mDatabase = std::make_unique<Database>(mContext, mConfig.postgres);
    // mThreads.emplace_back(&WebServerDatabaseWorker::run, mDatabase.get());

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
        for (auto & participantPtr : mParticipants)
            participantPtr->quit();
        mTCPAcceptor.close();
        mWebAcceptor.close();
    });
}

void WebServer::tcpAccept() {
    mTCPAcceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (ec) {
            log_error().field("message", ec.message()).msg("Received error code in async_accept");
        }
        else {
            auto connection = std::make_shared<NetworkConnection>(std::move(socket));

            connection->asyncAccept(boost::asio::bind_executor(mStrand, [this, connection](boost::system::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Received error code in connection.asyncAccept");
                    return;
                }

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

                auto participant = std::make_shared<WebParticipant>(mContext, std::move(connection), *this, *mDatabase);
                log_info().msg("Accepted web socket");
            }));

            // auto websocket = std::make_shared<>(std::move(socket));

            // connection->asyncAccept(boost::asio::bind_executor(mStrand, [this, connection](boost::system::error_code ec) {
            //     log_debug().msg("Async accept handler called");
            //     if (ec) {
            //         log_error().field("message", ec.message()).msg("Received error code in connection.asyncAccept");
            //         return;
            //     }

            //     auto participant = std::make_shared<TCPParticipant>(mContext, std::move(connection), *this, *mDatabase);
            //     participant->asyncAuth();
            //     mParticipants.insert(std::move(participant));
            // }));
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

void WebServer::assignWebName(std::shared_ptr<TCPParticipant> participant, std::string webName) {
    log_debug().field("webName", webName).msg("Assigning web name to participant");
}

void WebServer::work() {
    mContext.run();
}

void WebServer::obtainTournament(const std::string &webName, ObtainTournamentCallback callback) {
    mStrand.dispatch([this, webName, callback]() {
        auto it = mLoadedTournaments.find(webName);
        std::shared_ptr<LoadedTournament> tournament;
        if (it != mLoadedTournaments.end()) {
            // TODO: Kick existing participant if any
            tournament = it->second;
        }
        else {
            tournament = std::make_shared<LoadedTournament>(webName, mContext);
            mLoadedTournaments.insert({webName, tournament});
        }

        callback(std::move(tournament));
    });
}

