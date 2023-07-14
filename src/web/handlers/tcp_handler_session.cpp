#include "core/logger.hpp"
#include "web/handlers/tcp_handler.hpp"
#include "web/handlers/tcp_handler_session.hpp"

TCPHandlerSession::TCPHandlerSession(boost::asio::io_context &context, Logger &logger, TCPHandler &tcp_handler, std::unique_ptr<NetworkConnection> connection)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
    , mTCPHandler(tcp_handler)
    , mConnection(std::move(connection))
{}


void TCPHandlerSession::async_close() {

}

void TCPHandlerSession::async_listen() {

}
