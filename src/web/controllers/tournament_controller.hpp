#pragma once

#include <functional>
#include <boost/asio/io_context_strand.hpp>

#include "core/web/web_types.hpp"
#include "web/controllers/tournament_controller_session.hpp"
#include "web/gateways/meta_service_gateway.hpp"
#include "web/gateways/storage_gateway.hpp"
#include "web/models/tournament_meta.hpp"
#include "web/models/user_meta.hpp"

class Logger;

class TournamentController {
public:
    TournamentController(boost::asio::io_context &context, Logger &logger, const Config &config, StorageGateway &storageGateway, MetaServiceGateway &metaServiceGateway);

    typedef std::function<void (std::optional<Error>, std::shared_ptr<TournamentControllerSession>)> SubscribeTournamentCallback;
    // asyncSubscribeTournament returns a tournament session. If the tournament is
    // already loaded, then the existing session will be returned.  Otherwise,
    // it will be read from storage.
    void asyncSubscribeTournament(std::shared_ptr<WebHandlerSession> webSession, const std::string &tournamentID, SubscribeTournamentCallback);

    typedef std::function<void (std::optional<Error>, std::shared_ptr<TournamentControllerSession>)> AcquireTournamentCallback;
    // asyncAcquireTournament acquires ownership of a tournament session. if the
    // tournament is already loaded, then the existing session will be returned
    // and any TCP participant will be kicked. If not, then an empty session
    // will be returned. An error is returned if the tournament is not owned by
    // the given userID.
    void asyncAcquireTournament(std::shared_ptr<TCPHandlerSession> tcpSession, const std::string &tournamentID, const UserMeta &user, const UserCredentials &userCredentials, AcquireTournamentCallback callback);

    typedef std::function<void (std::optional<Error>, std::shared_ptr<std::vector<TournamentMeta>>, std::shared_ptr<std::vector<TournamentMeta>>)> ListTournamentsCallback;
    // asyncListTournaments lists upcoming and past tournaments.
    void asyncListTournaments(ListTournamentsCallback callback);

    void asyncClose();

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Logger &mLogger;
    const Config &mConfig;
    StorageGateway &mStorageGateway;
    MetaServiceGateway &mMetaServiceGateway;

    std::unordered_map<std::string, std::shared_ptr<TournamentControllerSession>> mTournamentSessions;
};
