#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>

#include "core/log.hpp"

class TCPParticipantHandler {
public:
    TCPParticipantHandler(Logger &logger, boost::asio::io_context &context);

private:
    boost::asio::io_context &mContext;
    Logger &mLogger;
};
