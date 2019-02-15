#pragma once

#include <boost/asio.hpp>

class WebParticipant {
public:
    WebParticipant(boost::asio::io_context &context);

private:
    boost::asio::io_context& mContext;
    boost::asio::io_context::strand mStrand;
};
