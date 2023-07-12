#include "web/handlers/tcp_handler.hpp"

TCPHandler::TCPHandler(boost::asio::io_context &context, Logger &logger)
    : mContext(context)
    , mLogger(logger)
{ }

void TCPHandler::async_listen() {

}

void TCPHandler::close() {

}
