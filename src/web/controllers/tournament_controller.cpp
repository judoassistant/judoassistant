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
    , mConfig(config)
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
                boost::asio::dispatch(mContext, std::bind(callback, std::nullopt, tournamentSession));
            }));
            return;
        }

        // Check storage
        mStorageGateway.asyncGetTournament(tournamentID, [this, webSession, tournamentID, callback](std::optional<Error> error, WebTournamentStore *tournamentPtr, std::chrono::milliseconds clockDiff) {
            if (error) {
                boost::asio::post(mContext, std::bind(callback, error, nullptr));
                return;
            }
            auto tournamentSession = std::make_shared<TournamentControllerSession>(mContext, mLogger, mConfig, mStorageGateway, tournamentID, std::unique_ptr<WebTournamentStore>(tournamentPtr), clockDiff);
            tournamentSession->asyncAddWebSession(webSession, boost::asio::bind_executor(mStrand, [this, callback, tournamentSession]() {
                boost::asio::dispatch(mContext, std::bind(callback, std::nullopt, tournamentSession));
            }));
            return;
        });
    });
}

void TournamentController::asyncAcquireTournament(std::shared_ptr<TCPHandlerSession> tcpSession, const std::string &shortName, int userID, AcquireTournamentCallback callback) {
    boost::asio::dispatch(mStrand, [this, shortName, userID, tcpSession, callback]() {
        mMetaServiceGateway.asyncGetTournament(shortName, [this, shortName, userID, tcpSession, callback](std::optional<Error> error, std::shared_ptr<TournamentMeta> tournament) {
            if (error && error->code != ErrorCode::NotFound) {
                boost::asio::post(mContext, std::bind(callback, error, nullptr));
                return;
            }

            if (!error && tournament->owner != userID) {
                // Tournament already exists and is owned by another user
                auto error = std::make_optional<Error>(ErrorCode::Unauthorized, "tournament is owner by another user");
                boost::asio::post(mContext, std::bind(callback, error, nullptr));
                return;
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
                boost::asio::post(mContext, std::bind(callback, std::nullopt, tournamentSession));
            }));
        });
    });
}

void TournamentController::asyncListTournaments(ListTournamentsCallback callback) {
    mMetaServiceGateway.asyncListPastTournaments([this, callback](std::optional<Error> error, std::shared_ptr<std::vector<TournamentMeta>> pastTournaments) {
        if (error) {
            boost::asio::post(mContext, std::bind(callback, error, nullptr, nullptr));
            return;
        }

        mMetaServiceGateway.asyncListUpcomingTournaments([this, callback, pastTournaments](std::optional<Error> error, std::shared_ptr<std::vector<TournamentMeta>> upcomingTournaments) {
            if (error) {
                boost::asio::post(mContext, std::bind(callback, error, nullptr, nullptr));
                return;
            }

            boost::asio::post(mContext, std::bind(callback, std::nullopt, std::move(pastTournaments), std::move(upcomingTournaments)));
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
