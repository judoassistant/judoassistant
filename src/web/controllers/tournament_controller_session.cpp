#include "web/controllers/tournament_controller_session.hpp"
#include "core/logger.hpp"

TournamentControllerSession::TournamentControllerSession(boost::asio::io_context &context, Logger &logger)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
{}
