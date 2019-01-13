#include "log.hpp"

#include "web/web_server.hpp"
#include "config/web_server.hpp"

void WebServer::run() {
    mDatabaseConnection = std::make_unique<pqxx::connection>(Config::POSTGRES_CONFIG);
    for (size_t i = 0; i < Config::WORKER_COUNT; ++i) {
        auto worker = std::make_unique<WebServerWorker>();
        mThreads.emplace_back(&WebServerWorker::run, worker.get());
        mWorkers.push_back(std::move(worker));
    }

    // done
    log_debug().msg("Running main thread");

    for (std::thread &thread : mThreads)
        thread.join();
}

void WebServer::quit() {

}

