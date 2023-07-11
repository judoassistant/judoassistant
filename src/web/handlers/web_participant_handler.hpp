#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>

#include "core/log.hpp"

class WebParticipantHandler {
public:
    WebParticipantHandler(Logger &logger, boost::asio::io_context &context);

private:
    boost::asio::io_context &mContext;
    Logger &mLogger;
};
