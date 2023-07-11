#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>

#include "core/log.hpp"

struct GetTournamentResponse {
    std::string tournament_owner;
    std::string tournament_name;
    std::string tournament_location;
    std::string tournament_date;
};

// MetaServiceGateway wraps the judoassistant-meta service endpoints.
class MetaServiceGateway {
public:
    MetaServiceGateway(boost::asio::io_context &context, Logger &logger);

    struct ListTournamentsResponse {
    };
    typedef std::function<void (ListTournamentsResponse)> ListTournamentsCallback;

    // ListTournaments lists upcoming and past tournaments.
    void ListTournaments(ListTournamentsCallback callback);

    // bool AuthenticateUser(const std::string &username, const std::string &password);

    // void GetTournament(const std::string &tournament_name);

private:
    boost::asio::io_context &mContext;
    Logger &mLogger;
};
