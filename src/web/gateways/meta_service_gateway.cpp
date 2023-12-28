#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/asio/connect.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/system/errc.hpp>

#include "core/web/web_types.hpp"
#include "web/gateways/meta_service_gateway.hpp"
#include "web/models/tournament_meta.hpp"
#include "web/models/user_meta.hpp"

MetaServiceGateway::MetaServiceGateway(boost::asio::io_context &context, Logger &logger, const Config &config)
    : mContext(context)
    , mLogger(logger)
    , mMapper()
    , mConfig(config)
    , mResolver(mContext)
{}

void MetaServiceGateway::asyncListUpcomingTournaments(ListTournamentsCallback callback) {
    auto request = buildRequest("tournaments/past", boost::beast::http::verb::get, std::nullopt, std::nullopt);
    asyncPerformRequest(std::move(request), [this, callback](std::optional<Error> error, std::shared_ptr<std::string> body){
        if (error) {
            boost::asio::post(mContext, std::bind(callback, error, nullptr));
            return;
        }

        auto tournaments = mMapper.mapTournamentListResponse(*body);
        auto tournamentsPtr = std::make_shared<std::vector<TournamentMeta>>(std::move(tournaments));
        boost::asio::post(mContext, std::bind(callback, std::nullopt, std::move(tournamentsPtr)));
    });
}

void MetaServiceGateway::asyncListPastTournaments(ListTournamentsCallback callback) {
    auto request = buildRequest("tournaments/past", boost::beast::http::verb::get, std::nullopt, std::nullopt);
    asyncPerformRequest(std::move(request), [this, callback](std::optional<Error> error, std::shared_ptr<std::string> body){
        if (error) {
            boost::asio::post(mContext, std::bind(callback, error, nullptr));
            return;
        }

        auto tournaments = mMapper.mapTournamentListResponse(*body);
        auto tournamentsPtr = std::make_shared<std::vector<TournamentMeta>>(std::move(tournaments));
        boost::asio::post(mContext, std::bind(callback, std::nullopt, std::move(tournamentsPtr)));
    });
}

void MetaServiceGateway::asyncGetTournament(const std::string &shortName, GetTournamentCallback callback) {
    const auto url = std::string("/tournaments/") + shortName;
    auto request = buildRequest(url, boost::beast::http::verb::get, std::nullopt, std::nullopt);
    asyncPerformRequest(std::move(request), [this, callback](std::optional<Error> error, std::shared_ptr<std::string> body){
        if (error) {
            boost::asio::post(mContext, std::bind(callback, error, nullptr));
            return;
        }

        auto tournament = mMapper.mapTournamentResponse(*body);
        auto tournamentPtr = std::make_shared<TournamentMeta>(tournament);
        boost::asio::post(mContext, std::bind(callback, std::nullopt, std::move(tournamentPtr)));
    });
}

void MetaServiceGateway::asyncCreateTournament(const std::string &shortName, const UserCredentials &user, CreateTournamentCallback callback) {
    const auto body = mMapper.mapTournamentCreateRequest(shortName);
    auto request = buildRequest("/tournaments", boost::beast::http::verb::post, body, user);
    asyncPerformRequest(std::move(request), [this, callback](std::optional<Error> error, std::shared_ptr<std::string> body){
        if (error) {
            boost::asio::post(mContext, std::bind(callback, error, nullptr));
            return;
        }

        auto tournament = mMapper.mapTournamentResponse(*body);
        auto tournamentPtr = std::make_shared<TournamentMeta>(tournament);
        boost::asio::post(mContext, std::bind(callback, std::nullopt, std::move(tournamentPtr)));
    });
}

void MetaServiceGateway::asyncUpdateTournament(const TournamentMeta &tournament, const UserCredentials &user, CreateTournamentCallback callback) {
    const auto body = mMapper.mapTournamentUpdateRequest(tournament);
    const auto url = std::string("/tournaments/") + tournament.shortName;
    auto request = buildRequest(url, boost::beast::http::verb::put, body, user);
    asyncPerformRequest(std::move(request), [this, callback](std::optional<Error> error, std::shared_ptr<std::string> body){
        if (error) {
            boost::asio::post(mContext, std::bind(callback, error, nullptr));
            return;
        }

        auto tournament = mMapper.mapTournamentResponse(*body);
        auto tournamentPtr = std::make_shared<TournamentMeta>(tournament);
        boost::asio::post(mContext, std::bind(callback, std::nullopt, std::move(tournamentPtr)));
    });
}

void MetaServiceGateway::asyncAuthenticateUser(const UserCredentials &user, AuthenticateUserCallback callback) {
    auto request = buildRequest("/users/me", boost::beast::http::verb::get, std::nullopt, user);
    asyncPerformRequest(std::move(request), [this, callback](std::optional<Error> error, std::shared_ptr<std::string> body){
        if (error) {
            boost::asio::post(mContext, std::bind(callback, error, nullptr));
            return;
        }

        auto user = mMapper.mapUserAuthenticateResponse(*body);
        auto userPtr = std::make_shared<UserMeta>(user);
        boost::asio::post(mContext, std::bind(callback, std::nullopt, std::move(userPtr)));
    });
}

// basicAuthHeader returns the base64 encoded basic auth header for a set of user credentials.
std::string basicAuthHeader(const UserCredentials &user) {
    const auto credentials = user.email + ":" + user.password;

    auto encodedCredentials = new char[boost::beast::detail::base64::encoded_size(credentials.size())];
    const auto encodedCredentialsSize = boost::beast::detail::base64::encode(encodedCredentials, credentials.c_str(), credentials.size());
    const auto header = std::string("Basic ") + std::string(encodedCredentials, encodedCredentials+encodedCredentialsSize);
    delete[] encodedCredentials;

    return header;
}

std::shared_ptr<boost::beast::http::request<boost::beast::http::string_body>> MetaServiceGateway::buildRequest(const std::string &path, boost::beast::http::verb verb, const std::optional<std::string> &body, const std::optional<UserCredentials> &user) {
    auto request = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>(
        boost::beast::http::request<boost::beast::http::string_body>{verb, path, 11}
    );
    request->set(boost::beast::http::field::host, mConfig.metaServiceHost);
    request->set(boost::beast::http::field::user_agent, "judoassistant-web");
    if (user) {
        request->set(boost::beast::http::field::authorization, basicAuthHeader(*user));
    }
    if (body) {
        request->set(boost::beast::http::field::content_type, "application/json");
        request->body() = *body;
        request->prepare_payload();
    }
    return request;
}

void MetaServiceGateway::asyncPerformRequest(std::shared_ptr<boost::beast::http::request<boost::beast::http::string_body>> request, const HTTPRequestCallback callback) {
    boost::asio::post(mContext, [this, callback, request]() {
        // Resolve host
        mResolver.async_resolve(mConfig.metaServiceHost, std::to_string(mConfig.metaServicePort), [this, callback, request](boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
            if (ec) {
                boost::asio::post(mContext, std::bind(callback, Error::wrapBoostSystemError(ec, "unable to resolve meta service hostname"), nullptr));
                return;
            }

            // Establish connection
            auto stream = std::make_shared<boost::beast::tcp_stream>(mContext);
            stream->expires_after(std::chrono::seconds(30));
            stream->async_connect(results, [this, callback, stream, request](boost::system::error_code ec, boost::asio::ip::tcp::resolver::endpoint_type /* endpointType */) {
                if (ec) {
                    boost::asio::post(mContext, std::bind(callback, Error::wrapBoostSystemError(ec, "unable to connect to meta service"), nullptr));
                    return;
                }

                boost::beast::http::async_write(*stream, *request, [this, callback, stream, request](boost::system::error_code ec, std::size_t /* bytesTransferred */) {
                    if (ec) {
                        boost::asio::post(mContext, std::bind(callback, Error::wrapBoostSystemError(ec, "unable to write to meta service"), nullptr));
                        return;
                    }

                    // Read response
                    auto buffer = std::make_shared<boost::beast::flat_buffer>();
                    auto response = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
                    boost::beast::http::async_read(*stream, *buffer, *response, [this, callback, stream, response](boost::system::error_code ec, std::size_t /* bytesTransferred */) {
                        if (ec) {
                            boost::asio::post(mContext, std::bind(callback, Error::wrapBoostSystemError(ec, "unable to read from meta service"), nullptr));
                            return;
                        }

                        auto error = Error::wrapHTTPStatus(response->result_int(), "unable to execute post request");
                        auto body = std::make_shared<std::string>(response->body());
                        boost::asio::post(mContext, std::bind(callback, error, std::move(body)));

                        // Gracefully close the socket
                        stream->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                        if(ec && ec != boost::beast::errc::not_connected) {
                            // not_connected happens sometimes so don't bother reporting it.
                            mLogger.warn("Unable to close meta service stream", LoggerField(ec));
                        }
                    });
                });
            });

        });
    });
}
