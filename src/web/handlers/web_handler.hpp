#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>

#include "web/config/config.hpp"
#include "core/log.hpp"

class WebHandler {
public:
    WebHandler(boost::asio::io_context &context, Logger &logger, const Config &config);
    void async_listen();
    void close();

private:
    boost::asio::io_context &mContext;
    Logger &mLogger;
    boost::asio::io_context::strand mStrand;

    boost::asio::ip::tcp::endpoint mEndpoint;
    boost::asio::ip::tcp::acceptor mAcceptor;
};
