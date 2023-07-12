#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>

#include "core/log.hpp"

class WebParticipantHandler {
public:
    WebParticipantHandler(boost::asio::io_context &context, Logger &logger);
    void listen();

private:
    boost::asio::io_context &mContext;
    Logger &mLogger;
};
