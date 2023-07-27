#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/system/errc.hpp>

#include "core/web/web_types.hpp"
#include "web/controllers/tournament_controller.hpp"
#include "core/logger.hpp"
#include "web/controllers/tournament_controller_session.hpp"

TournamentController::TournamentController(boost::asio::io_context &context, Logger &logger)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
{}

void TournamentController::asyncSubscribeTournament(std::shared_ptr<WebHandlerSession> webSession, const std::string &tournamentID, SubscribeTournamentCallback callback) {
    boost::asio::dispatch(mStrand, [this, tournamentID, webSession, callback]() {
        // Check existing sessions
        auto it = mTournamentSessions.find(tournamentID);
        if (it != mTournamentSessions.end()) {
            auto tournamentSession = it->second;
            tournamentSession->asyncAddWebSession(webSession, boost::asio::bind_executor(mStrand, [this, callback, tournamentSession]() {
                boost::asio::dispatch(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::success), tournamentSession));
            }));
            return;
        }

        // TODO: Read from storage gateway

        // Return not found
        boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory), nullptr));
    });
}

void TournamentController::asyncAcquireTournament(std::shared_ptr<TCPHandlerSession> tcpSession, const std::string &tournamentID, int userID, AcquireTournamentCallback callback) {
    boost::asio::dispatch(mStrand, [this, tournamentID, tcpSession, callback]() {
        auto it = mTournamentSessions.find(tournamentID);
        std::shared_ptr<TournamentControllerSession> tournamentSession;
        if (it != mTournamentSessions.end()) {
            tournamentSession = it->second;
        } else {
            tournamentSession = mTournamentSessions[tournamentID] = std::make_shared<TournamentControllerSession>(mContext, mLogger);
        }

        // TODO: Check meta-service user information
        tournamentSession->asyncUpsertTCPSession(tcpSession, boost::asio::bind_executor(mStrand, [this, callback, tournamentSession]() {
            boost::asio::post(mContext, std::bind(callback, WebNameRegistrationResponse::SUCCESSFUL, tournamentSession));
        }));
    });
}
