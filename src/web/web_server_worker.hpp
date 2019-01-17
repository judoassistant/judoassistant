#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers

// Responsible for handling connections to a set of tournaments
class WebServerWorker {
public:
    WebServerWorker(boost::asio::io_context &masterContext);

    void run();
    void quit();

private:
    boost::asio::io_context mContext;
    boost::asio::io_context &mMasterContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;
};
