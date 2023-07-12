#include "web/handlers/tcp_participant_handler.hpp"

TCPParticipantHandler::TCPParticipantHandler(boost::asio::io_context &context, Logger &logger)
    : mContext(context)
    , mLogger(logger)
{ }

void TCPParticipantHandler::listen() {

}
