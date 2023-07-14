#pragma once

#include <memory>
#include <boost/asio/io_context_strand.hpp>

#include "core/network/network_connection.hpp"

class TCPHandler;
class Logger;

// TCPHandlerSession represents a stateful TCP socket session. Sessions are
// created whenever a new TCP connection is established from the JudoAssistant
// main application.
class TCPHandlerSession : public std::enable_shared_from_this<TCPHandlerSession> {
public:
    TCPHandlerSession(boost::asio::io_context &context, Logger &logger, TCPHandler &tcp_handler, std::unique_ptr<NetworkConnection> connection);

    void async_close();
    void async_listen();

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Logger &mLogger;
    TCPHandler &mTCPHandler;
    std::unique_ptr<NetworkConnection> mConnection;
};
