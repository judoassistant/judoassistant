#include "core/id.hpp"
#include "core/log.hpp"
#include "core/web/web_types.hpp"
#include "web/web_server.hpp"

// TODO: Move into a json file and read on runtime
constexpr size_t WORKER_COUNT = 2; // Number of threads to handle tournaments
constexpr int WEB_PORT = 8000;

using boost::asio::ip::tcp;

WebServer::WebServer()
    : mContext()
    , mEndpoint(tcp::v4(), WEB_PORT)
    , mAcceptor(mContext, mEndpoint)
{
    tcpAccept();
}

void WebServer::run() {
    log_info().msg("Launching database worker");
    mDatabaseWorker = std::make_unique<WebServerDatabaseWorker>(mContext);
    mThreads.emplace_back(&WebServerDatabaseWorker::run, mDatabaseWorker.get());

    log_info().field("WORKER_COUNT", WORKER_COUNT).msg("Launching workers");
    for (size_t i = 0; i < WORKER_COUNT; ++i) {
        auto worker = std::make_unique<WebServerWorker>(mContext);
        mThreads.emplace_back(&WebServerWorker::run, worker.get());
        mWorkers.push_back(std::move(worker));
    }

    // log_info().msg("Waiting for clients");
    // mDatabaseWorker->asyncRegisterUser("svendcsvendsen@gmail.com", "password", [this](UserRegistrationResponse response, const WebToken &token) {});

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
                    auto participant = std::make_unique<WebParticipant>(std::move(connection), *this, *mDatabaseWorker);
                    mParticipants.insert(std::move(participant));
                }
            });
        }

        if (mAcceptor.is_open())
            tcpAccept();
    });
}

void WebServer::leave(std::shared_ptr<WebParticipant> participant) {
    mParticipants.erase(participant);
}

void WebServer::assignWebName(std::shared_ptr<WebParticipant> participant, std::string webName) {
    log_debug().msg("Assigning web name to participant");
}

