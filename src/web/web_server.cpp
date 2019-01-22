#include "log.hpp"

#include "web/web_server.hpp"
#include "id.hpp"
#include "config/web_server.hpp"
#include "config/web.hpp"
#include "web/web_types.hpp"

using boost::asio::ip::tcp;

WebServer::WebServer()
    : mContext()
    , mEndpoint(tcp::v4(), Config::WEB_PORT)
    , mAcceptor(mContext, mEndpoint)
{
    tcpAccept();
}

void WebServer::run() {
    log_info().msg("Launching database worker");
    mDatabaseWorker = std::make_unique<WebServerDatabaseWorker>(mContext);
    mThreads.emplace_back(&WebServerDatabaseWorker::run, mDatabaseWorker.get());

    log_info().field("WORKER_COUNT", Config::WORKER_COUNT).msg("Launching workers");
    for (size_t i = 0; i < Config::WORKER_COUNT; ++i) {
        auto worker = std::make_unique<WebServerWorker>(mContext);
        mThreads.emplace_back(&WebServerWorker::run, worker.get());
        mWorkers.push_back(std::move(worker));
    }

    log_info().msg("Waiting for clients");
    mDatabaseWorker->asyncRegisterUser("svendcsvendsen@gmail.com", "password", [this](UserRegistrationResponse response, const Token &token) {});

    mContext.run();
    log_info().msg("Joining threads");

    for (std::thread &thread : mThreads)
        thread.join();
}

void WebServer::quit() {
    mAcceptor.close();
    mDatabaseWorker->quit();
    for (auto &worker : mWorkers)
        worker->quit();
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
                    auto participant = std::make_unique<WebParticipant>(std::move(connection), *this);
                    mParticipants.insert(std::move(participant));
                }
            });
        }

        if (mAcceptor.is_open())
            tcpAccept();
    });
}
