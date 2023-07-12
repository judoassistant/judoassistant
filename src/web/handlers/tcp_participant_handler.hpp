#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>

#include "core/log.hpp"

class TCPParticipantHandler {
public:
    TCPParticipantHandler(boost::asio::io_context &context, Logger &logger);
    void run();

private:
    boost::asio::io_context &mContext;
    Logger &mLogger;
};
