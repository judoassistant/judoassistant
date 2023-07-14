#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "web/handlers/tcp_handler_session.hpp"

class TournamentController;
class Logger;
struct Config;

class TCPHandler {
public:
    TCPHandler(boost::asio::io_context &context, Logger &logger, const Config &config, TournamentController &tournamentController);
    void asyncListen();
    void asyncClose();

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;

    Logger &mLogger;
    TournamentController &mTournamentController;

    boost::asio::ip::tcp::endpoint mEndpoint;
    boost::asio::ip::tcp::acceptor mAcceptor;
    std::vector<std::shared_ptr<TCPHandlerSession>> mSessions;
};
