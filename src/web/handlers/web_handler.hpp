#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>

#include "web/handlers/web_handler_session.hpp"
#include "core/log.hpp"

class TournamentController;
class Logger;
struct Config;

class WebHandler {
public:
    WebHandler(boost::asio::io_context &context, Logger &logger, const Config &config, TournamentController &tournamentController);
    void asyncListen();
    void asyncClose();

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;

    Logger &mLogger;
    TournamentController &mTournamentController;

    boost::asio::ip::tcp::endpoint mEndpoint;
    boost::asio::ip::tcp::acceptor mAcceptor;
    std::vector<std::shared_ptr<WebHandlerSession>> mSessions;
};
