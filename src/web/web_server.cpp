#include "core/id.hpp"
#include "core/log.hpp"
#include "core/web/web_types.hpp"
#include "web/web_server.hpp"

using boost::asio::ip::tcp;

WebServer::WebServer(const Config &config)
    : mConfig(config)
    , mContext()
    , mEndpoint(tcp::v4(), config.port)
    , mAcceptor(mContext, mEndpoint)
{
    tcpAccept();
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
    mAcceptor.close();
    // for (auto &worker : mWorkers)
    //     worker->quit();
}

void WebServer::tcpAccept() {
    mAcceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (ec) {
            log_error().field("message", ec.message()).msg("Received error code in async_accept");
        }
        else {
            auto connection = std::make_shared<NetworkConnection>(std::move(socket));

            connection->asyncAccept([this, connection](boost::system::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Received error code in connection.asyncAccept");
                }
                else {
                    auto participant = std::make_unique<TCPParticipant>(std::move(connection), *this, *mDatabase);
                    mParticipants.insert(std::move(participant));
                }
            });
        }

        if (mAcceptor.is_open())
            tcpAccept();
    });
}

void WebServer::leave(std::shared_ptr<TCPParticipant> participant) {
    mParticipants.erase(participant);
}

void WebServer::assignWebName(std::shared_ptr<TCPParticipant> participant, std::string webName) {
    log_debug().msg("Assigning web name to participant");
}

void WebServer::work() {
    mContext.run();
}
