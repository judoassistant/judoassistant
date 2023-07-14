#include "core/logger.hpp"
#include "web/handlers/tcp_handler.hpp"
#include "web/handlers/tcp_handler_session.hpp"

TCPHandlerSession::TCPHandlerSession(boost::asio::io_context &context, Logger &logger, TCPHandler &tcpHandler, std::unique_ptr<NetworkConnection> connection)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
    , mTCPHandler(tcpHandler)
    , mConnection(std::move(connection))
{}


void TCPHandlerSession::asyncClose() {

}

void TCPHandlerSession::asyncListen() {

}
