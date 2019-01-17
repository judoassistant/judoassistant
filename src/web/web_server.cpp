#include "log.hpp"

#include "web/web_server.hpp"
#include "config/web_server.hpp"
#include "web/web_types.hpp"

WebServer::WebServer()
    : mContext()
    , mWorkGuard(boost::asio::make_work_guard(mContext))
{

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
    mWorkGuard.reset();
    mDatabaseWorker->quit();
    for (auto &worker : mWorkers)
        worker->quit();
}

