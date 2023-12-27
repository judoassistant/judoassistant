#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/system/errc.hpp>

#include "core/logger.hpp"
#include "core/web/web_types.hpp"
#include "web/controllers/tournament_controller.hpp"
#include "web/controllers/tournament_controller_session.hpp"
#include "web/web_tournament_store.hpp"

TournamentController::TournamentController(boost::asio::io_context &context, Logger &logger, const Config &config, StorageGateway &storageGateway, MetaServiceGateway &metaServiceGateway)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
    , mStorageGateway(storageGateway)
    , mMetaServiceGateway(metaServiceGateway)
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

        // Check storage
        mStorageGateway.asyncGetTournament(tournamentID, [this, webSession, tournamentID, callback](boost::system::error_code ec, WebTournamentStore *tournamentPtr, std::chrono::milliseconds clockDiff) {
            if (ec) {
                // TODO: Check error code
                // Return not found
                boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory), nullptr));
                return;
            }
            auto tournamentSession = std::make_shared<TournamentControllerSession>(mContext, mLogger, mStorageGateway, tournamentID, std::unique_ptr<WebTournamentStore>(tournamentPtr), clockDiff);
            tournamentSession->asyncAddWebSession(webSession, boost::asio::bind_executor(mStrand, [this, callback, tournamentSession]() {
                boost::asio::dispatch(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::success), tournamentSession));
            }));
            return;
        });
    });
}

void TournamentController::asyncAcquireTournament(std::shared_ptr<TCPHandlerSession> tcpSession, const std::string &shortName, int userID, AcquireTournamentCallback callback) {
    boost::asio::dispatch(mStrand, [this, shortName, userID, tcpSession, callback]() {
        mMetaServiceGateway.asyncGetTournament(shortName, [this, shortName, userID, tcpSession, callback](boost::system::error_code ec, std::shared_ptr<TournamentMeta> tournament) {
            if (ec && ec.value() != boost::system::errc::no_such_file_or_directory) {
                boost::asio::post(mContext, std::bind(callback, ec, nullptr));
                return;
            }

            if (ec.value() != boost::system::errc::no_such_file_or_directory) {
                if (tournament->owner != userID) {
                    // Tournament already exists and is owned by another user
                    boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::permission_denied), nullptr));
                    return;
                }
            }

            // TODO: Upsert tournament afterwards

            auto it = mTournamentSessions.find(shortName);
            std::shared_ptr<TournamentControllerSession> tournamentSession;
            if (it != mTournamentSessions.end()) {
                tournamentSession = it->second;
            } else {
                tournamentSession = mTournamentSessions[shortName] = std::make_shared<TournamentControllerSession>(mContext, mLogger, mConfig, mStorageGateway, shortName);
            }

            tournamentSession->asyncUpsertTCPSession(tcpSession, boost::asio::bind_executor(mStrand, [this, callback, tournamentSession]() {
                boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::success), tournamentSession));
            }));
        });
    });
}

void TournamentController::asyncListTournaments(ListTournamentsCallback callback) {
    mMetaServiceGateway.asyncListPastTournaments([this, callback](boost::system::error_code ec, std::shared_ptr<std::vector<TournamentMeta>> pastTournaments) {
        if (ec) {
            boost::asio::post(mContext, std::bind(callback, ec, nullptr, nullptr));
            return;
        }

        mMetaServiceGateway.asyncListUpcomingTournaments([this, callback, pastTournaments](boost::system::error_code ec, std::shared_ptr<std::vector<TournamentMeta>> upcomingTournaments) {
            if (ec) {
                boost::asio::post(mContext, std::bind(callback, ec, nullptr, nullptr));
                return;
            }

            boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::success), std::move(pastTournaments), std::move(upcomingTournaments)));
        });
    });
}

void TournamentController::asyncClose() {
    boost::asio::post(boost::asio::bind_executor(mStrand, [this]() {
        for (auto &p : mTournamentSessions) {
            // TODO: What if a session is removed?
            p.second->asyncClose();
        }
    }));
}
