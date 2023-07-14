#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>

#include "core/log.hpp"
#include "core/web/web_types.hpp"

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

    typedef std::function<void (WebTokenRequestResponse resp, std::optional<int> userID)> AuthenticateUserCallback;
    void asyncAuthenticateUser(const std::string &email, const std::string &password, AuthenticateUserCallback callback);

    // void GetTournament(const std::string &tournament_name);

private:
    boost::asio::io_context &mContext;
    Logger &mLogger;
};
