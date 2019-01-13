#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers
#include <pqxx/pqxx>

class WebServerDatabaseWorker {
public:
    WebServerDatabaseWorker();

    void run();
    void quit();

private:
    boost::asio::io_context mContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;
};
