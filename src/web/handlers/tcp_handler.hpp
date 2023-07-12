#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>

#include "core/log.hpp"

class TCPHandler {
public:
    TCPHandler(boost::asio::io_context &context, Logger &logger);
    void async_listen();
    void close();

private:
    boost::asio::io_context &mContext;
    Logger &mLogger;
};
