#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/system/detail/error_code.hpp>
#include <functional>

#include "core/log.hpp"
#include "core/web/web_types.hpp"
#include "web/config/config.hpp"
#include "web/models/tournament_meta.hpp"

struct GetTournamentResponse {
    std::string tournament_owner;
    std::string tournament_name;
    std::string tournament_location;
    std::string tournament_date;
};

// MetaServiceGateway wraps the judoassistant-meta service endpoints.
class MetaServiceGateway {
public:
    MetaServiceGateway(boost::asio::io_context &context, Logger &logger, const Config &config);

    typedef std::function<void (boost::system::error_code, std::shared_ptr<std::vector<TournamentMeta>>)> ListTournamentsCallback;

    // asyncListTournaments lists upcoming and past tournaments.
    void asyncListUpcomingTournaments(ListTournamentsCallback callback);
    void asyncListPastTournaments(ListTournamentsCallback callback);

    typedef std::function<void (WebTokenRequestResponse resp, std::optional<int> userID)> AuthenticateUserCallback;
    void asyncAuthenticateUser(const std::string &email, const std::string &password, AuthenticateUserCallback callback);

    // void GetTournament(const std::string &tournament_name);

private:
    boost::asio::io_context &mContext;
    Logger &mLogger;
    const Config &mConfig;
};
