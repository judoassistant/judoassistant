#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <memory>

class Logger;

// WebHandlerSession represents a stateful tournament session. Tournament
// sessions are created by the controller whenever tournaments are read or
// written by handler sessions.
class TournamentControllerSession : public std::enable_shared_from_this<TournamentControllerSession>{
public:
    TournamentControllerSession(boost::asio::io_context &context, Logger &logger);

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Logger &mLogger;
};
