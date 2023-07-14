#include <boost/asio/dispatch.hpp>

#include "core/web/web_types.hpp"
#include "web/controllers/tournament_controller.hpp"
#include "core/logger.hpp"

TournamentController::TournamentController(boost::asio::io_context &context, Logger &logger)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
{}

void TournamentController::asyncGetTournament(const std::string &tournamentID, GetTournamentCallback callback) {
    boost::asio::dispatch(mStrand, [this, tournamentID, callback]() {
        // Check existing sessions
        auto it = mTournamentSessions.find(tournamentID);
        if (it != mTournamentSessions.end()) {
            boost::asio::dispatch(mContext, std::bind(callback, it->second));
        }

        // TODO: Read from storage gateway

        // Return not found
        boost::asio::dispatch(mContext, std::bind(callback, nullptr));
    });
}

void TournamentController::asyncAcquireTournament(const std::string &tournamentID, const int userID, AcquireTournamentCallback callback) {
    boost::asio::dispatch(mStrand, [this, tournamentID, callback]() {
        // TODO: Implement
        boost::asio::dispatch(mContext, std::bind(callback, WebNameRegistrationResponse::OCCUPIED_OTHER_USER, nullptr));
    });
}
