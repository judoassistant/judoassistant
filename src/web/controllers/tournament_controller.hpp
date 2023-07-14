#pragma once

#include <functional>
#include <boost/asio/io_context_strand.hpp>

#include "web/controllers/tournament_controller_session.hpp"

class Logger;

class TournamentController {
public:
    TournamentController(boost::asio::io_context &context, Logger &logger);

    typedef std::function<void (std::shared_ptr<TournamentControllerSession>)> GetTournamentCallback;
    // asyncGetTournament returns a tournament session. The if tournament is
    // already loaded, then the existing session will be returned.  Otherwise,
    // it will be read from storage.
    void asyncGetTournament(const std::string &tournamentID, GetTournamentCallback);

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
