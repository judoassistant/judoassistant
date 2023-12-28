#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <functional>

#include "core/log.hpp"
#include "core/web/web_types.hpp"
#include "web/config/config.hpp"
#include "web/models/tournament_meta.hpp"
#include "web/gateways/meta_service_gateway_mapper.hpp"
#include "web/error.hpp"
#include "web/models/user_meta.hpp"

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
    void asyncCreateTournament(const std::string &shortName, const UserCredentials &user, CreateTournamentCallback callback);

    typedef std::function<void (std::optional<Error>, std::shared_ptr<TournamentMeta>)> UpdateTournamentCallback;
    void asyncUpdateTournament(const TournamentMeta &tournament, const UserCredentials &user, UpdateTournamentCallback callback);

    typedef std::function<void (std::optional<Error>, std::shared_ptr<UserMeta>)> AuthenticateUserCallback;
    void asyncAuthenticateUser(const UserCredentials &user, AuthenticateUserCallback callback);

private:
    typedef std::function<void (std::optional<Error>, std::shared_ptr<std::string>)> HTTPRequestCallback;
    void asyncPerformRequest(std::shared_ptr<boost::beast::http::request<boost::beast::http::string_body>> request, HTTPRequestCallback callback);
    std::shared_ptr<boost::beast::http::request<boost::beast::http::string_body>> buildRequest(const std::string &path, boost::beast::http::verb verb, const std::optional<std::string> &body, const std::optional<UserCredentials> &user);

    boost::asio::io_context &mContext;
    Logger &mLogger;
    MetaServiceGatewayMapper mMapper;
    const Config &mConfig;
    boost::asio::ip::tcp::resolver mResolver;
};
