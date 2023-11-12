#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/system/errc.hpp>

#include "core/web/web_types.hpp"
#include "web/gateways/meta_service_gateway.hpp"

MetaServiceGateway::MetaServiceGateway(boost::asio::io_context &context, Logger &logger, const Config &config)
    : mContext(context)
    , mLogger(logger)
    , mMapper()
    , mConfig(config)
    , mResolver(mContext)
{}

void MetaServiceGateway::asyncListUpcomingTournaments(ListTournamentsCallback callback) {
    asyncGetRequest("/tournaments/upcoming", [this, callback](boost::system::error_code ec, std::shared_ptr<std::string> body){
        if (ec) {
            boost::asio::post(mContext, std::bind(callback, ec, nullptr));
            return;
        }

        auto tournaments = mMapper.mapTournamentListResponse(*body);
        auto tournamentsPtr = std::make_shared<std::vector<TournamentMeta>>(std::move(tournaments));
        boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::success), std::move(tournamentsPtr)));
    });
}

void MetaServiceGateway::asyncListPastTournaments(ListTournamentsCallback callback) {
    asyncGetRequest("/tournaments/past", [this, callback](boost::system::error_code ec, std::shared_ptr<std::string> body){
        if (ec) {
            boost::asio::post(mContext, std::bind(callback, ec, nullptr));
            return;
        }

        auto tournaments = mMapper.mapTournamentListResponse(*body);
        auto tournamentsPtr = std::make_shared<std::vector<TournamentMeta>>(std::move(tournaments));
        boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::success), std::move(tournamentsPtr)));
    });
}

void MetaServiceGateway::asyncAuthenticateUser(const std::string &email, const std::string &password, AuthenticateUserCallback callback) {
    // TODO: Implement
    std::optional<int> userID = 1;
    boost::asio::post(mContext, std::bind(callback, WebTokenRequestResponse::SUCCESSFUL, userID));
}


void MetaServiceGateway::asyncGetRequest(const std::string &path, AsyncGetRequestCallback callback) {
    auto request = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>(
        boost::beast::http::request<boost::beast::http::string_body>{boost::beast::http::verb::get, path, 11}
        );
    request->set(boost::beast::http::field::host, mConfig.metaServiceHost);
    request->set(boost::beast::http::field::user_agent, "judoassistant-web");

    boost::asio::post(mContext, [this, callback, request]() {
        // Resolve host
        mResolver.async_resolve(mConfig.metaServiceHost, std::to_string(mConfig.metaServicePort), [this, callback, request](boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
            if (ec) {
                mLogger.error("Unable to resolve meta service hostname", LoggerField(ec));
                boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::network_down), nullptr));
                return;
            }

            // Establish connection
            auto stream = std::make_shared<boost::beast::tcp_stream>(mContext);
            stream->expires_after(std::chrono::seconds(30));
            stream->async_connect(results, [this, callback, stream, request](boost::system::error_code ec, boost::asio::ip::tcp::resolver::endpoint_type /* endpointType */) {
                if (ec) {
                    mLogger.error("Unable to connect to meta service", LoggerField(ec));
                    boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::network_down), nullptr));
                    return;
                }

                boost::beast::http::async_write(*stream, *request, [this, callback, stream, request](boost::system::error_code ec, std::size_t /* bytesTransferred */) {
                    if (ec) {
                        mLogger.error("Unable to write to meta service", LoggerField(ec));
                        boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::network_down), nullptr));
                        return;
                    }

                    // Read response
                    auto buffer = std::make_shared<boost::beast::flat_buffer>();
                    auto response = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
                    boost::beast::http::async_read(*stream, *buffer, *response, [this, callback, stream, response](boost::system::error_code ec, std::size_t /* bytesTransferred */) {
                        if (ec) {
                            mLogger.error("Unable to read from meta service", LoggerField(ec));
                            boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::network_down), nullptr));
                            return;
                        }

                        auto body = std::make_shared<std::string>(response->body());
                        boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(boost::system::errc::success), std::move(body)));

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
