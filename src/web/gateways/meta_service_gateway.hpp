#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http/verb.hpp>
#include <functional>

#include "core/log.hpp"
#include "core/web/web_types.hpp"
#include "web/config/config.hpp"
#include "web/models/tournament_meta.hpp"
#include "web/gateways/meta_service_gateway_mapper.hpp"
#include "web/error.hpp"

// MetaServiceGateway wraps the judoassistant-meta service endpoints.
class MetaServiceGateway {
public:
    MetaServiceGateway(boost::asio::io_context &context, Logger &logger, const Config &config);

    typedef std::function<void (std::optional<Error> error, std::shared_ptr<std::vector<TournamentMeta>>)> ListTournamentsCallback;
    void asyncListUpcomingTournaments(ListTournamentsCallback callback);
    void asyncListPastTournaments(ListTournamentsCallback callback);

    typedef std::function<void (std::optional<Error> error, std::shared_ptr<TournamentMeta>)> GetTournamentCallback;
    void asyncGetTournament(const std::string &shortName, GetTournamentCallback callback);

    typedef std::function<void (std::optional<Error>, std::shared_ptr<TournamentMeta>)> CreateTournamentCallback;
    void asyncCreateTournament(const std::string &shortName, CreateTournamentCallback callback);

    typedef std::function<void (std::optional<Error>, std::shared_ptr<TournamentMeta>)> UpdateTournamentCallback;
    void asyncUpdateTournament(const TournamentMeta &tournament, UpdateTournamentCallback callback);

    typedef std::function<void (std::optional<Error>, std::shared_ptr<UserMeta>)> AuthenticateUserCallback;
    void asyncAuthenticateUser(const std::string &email, const std::string &password, AuthenticateUserCallback callback);

private:
    typedef std::function<void (std::optional<Error>, std::shared_ptr<std::string>)> HTTPRequestCallback;
    void asyncRequest(const std::string &path, boost::beast::http::verb verb, HTTPRequestCallback callback);
    void asyncRequestWithBody(const std::string &path, boost::beast::http::verb verb, const std::string &body, HTTPRequestCallback callback);

    boost::asio::io_context &mContext;
    Logger &mLogger;
    MetaServiceGatewayMapper mMapper;
    const Config &mConfig;
    boost::asio::ip::tcp::resolver mResolver;
};
