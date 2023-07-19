#pragma once

#include <functional>
#include <boost/asio/io_context_strand.hpp>

#include "core/web/web_types.hpp"
#include "web/controllers/tournament_controller_session.hpp"

class Logger;

class TournamentController {
public:
    TournamentController(boost::asio::io_context &context, Logger &logger);

    typedef std::function<void (boost::system::error_code, std::shared_ptr<TournamentControllerSession>)> SubscribeTournamentCallback;
    // asyncSubscribeTournament returns a tournament session. If the tournament is
    // already loaded, then the existing session will be returned.  Otherwise,
    // it will be read from storage.
    void asyncSubscribeTournament(std::shared_ptr<WebHandlerSession> webSession, const std::string &tournamentID, SubscribeTournamentCallback);

    typedef std::function<void (WebNameRegistrationResponse resp, std::shared_ptr<TournamentControllerSession>)> AcquireTournamentCallback;
    // asyncAcquireTournament acquires ownership of a tournament session. if the
    // tournament is already loaded, then the existing session will be returned
    // and any TCP participant will be kicked. If not, then an empty session
    // will be returned. An error is returned if the tournament is not owned by
    // the given userID.
    void asyncAcquireTournament(std::shared_ptr<TCPHandlerSession> tcpSession, const std::string &tournamentID, int userID, AcquireTournamentCallback callback);

    struct ListTournamentsResponse {};
    typedef std::function<void (ListTournamentsResponse)> ListTournamentsCallback;
    // asyncListTournaments lists upcoming and past tournaments.
    void asyncListTournaments();

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Logger &mLogger;

    std::unordered_map<std::string, std::shared_ptr<TournamentControllerSession>> mTournamentSessions;
};
