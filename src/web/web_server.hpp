#pragma once

#include <thread>
#include <vector>
#include <pqxx/pqxx>

#include "core.hpp"
#include "web/web_server_worker.hpp"

class WebServer {
public:
    void run();
    void quit();

private:
    std::unique_ptr<pqxx::connection> mDatabaseConnection;
    std::vector<std::thread> mThreads;
    std::vector<std::unique_ptr<WebServerWorker>> mWorkers;
};
