#pragma once

#include <thread>
#include <vector>
#include <boost/asio.hpp> // TODO: Do not include boost convenience headers

#include "core.hpp"
#include "web/web_server_worker.hpp"
#include "web/web_server_database_worker.hpp"

class WebServer {
public:
    WebServer();

    void run();
    void quit();

private:
    boost::asio::io_context mContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;

    std::vector<std::thread> mThreads;
    std::unique_ptr<WebServerDatabaseWorker> mDatabaseWorker;
    std::vector<std::unique_ptr<WebServerWorker>> mWorkers;
};
