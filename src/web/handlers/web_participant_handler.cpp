#include "web/handlers/tcp_participant_handler.hpp"

WebParticipantHandler::WebParticipantHandler(Logger &logger, boost::asio::io_context &context)
    : mContext(context)
    , mLogger(logger)
{}
