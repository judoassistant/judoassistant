#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/system/errc.hpp>

#include "core/web/web_types.hpp"
#include "web/gateways/meta_service_gateway.hpp"

MetaServiceGateway::MetaServiceGateway(boost::asio::io_context &context, Logger &logger)
    : mContext(context)
    , mLogger(logger)
{}

void MetaServiceGateway::ListTournaments(ListTournamentsCallback callback) {
    // TODO: Implement
    // // Check command line arguments.
    // std::string host;
    // std::string port;
    // std::string target;

    // // These objects perform our I/O
    // boost::asio::ip::tcp::resolver resolver(mContext);
    // const auto results = resolver.resolve(host, port);

    // // Make the connection on the IP address we get from a lookup
    // boost::beast::tcp_stream stream(mContext);
    // stream.connect(results);

    // // Set up an HTTP GET request message
    // boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, target, 11};
    // req.set(boost::beast::http::field::host, host);
    // req.set(boost::beast::http::field::user_agent, "foo");

    // // Send the HTTP request to the remote host
    // boost::beast::http::write(stream, req);

    // // This buffer is used for reading and must be persisted
    // boost::beast::flat_buffer buffer;

    // // Declare a container to hold the response
    // boost::beast::http::response<boost::beast::http::dynamic_body> res;

    // // Receive the HTTP response
    // boost::beast::http::read(stream, buffer, res);

    // // Write the message to standard out
    // std::cout << res << std::endl;

    // // Gracefully close the socket
    // boost::beast::error_code ec;
    // stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

    // // not_connected happens sometimes
    // // so don't bother reporting it.
    // //
    // if(ec && ec != boost::beast::errc::not_connected)
    //     throw boost::beast::system_error{ec};


    ListTournamentsResponse resp;
    boost::asio::post(mContext, std::bind(callback, resp));

    // If we get here then the connection is closed gracefully
}

void MetaServiceGateway::asyncAuthenticateUser(const std::string &email, const std::string &password, AuthenticateUserCallback callback) {
    // TODO: Implement
    std::optional<int> userID = 1;
    boost::asio::post(mContext, std::bind(callback, WebTokenRequestResponse::SUCCESSFUL, userID));
}
