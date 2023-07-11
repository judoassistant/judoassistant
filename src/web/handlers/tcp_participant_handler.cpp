#include "web/handlers/tcp_participant_handler.hpp"

TCPParticipantHandler::TCPParticipantHandler(Logger &logger, boost::asio::io_context &context)
    : mContext(context)
    , mLogger(logger)
{ }
