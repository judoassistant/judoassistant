#include <boost/asio/connect.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include "core/log.hpp"
#include "core/network/ssl_socket.hpp"

SSLSocket::SSLSocket(boost::asio::io_context &context)
    : mContext(context)
    , mSSLContext(boost::asio::ssl::context::sslv23)
    , mSocket(mContext, mSSLContext)
{
    mSSLContext.set_default_verify_paths();
}

void SSLSocket::asyncConnect(const std::string &hostname, unsigned int port, ConnectHandler handler) {
    boost::asio::ip::tcp::resolver resolver(mContext);
    boost::asio::ip::tcp::resolver::results_type endpoints;

    try {
        endpoints = resolver.resolve(hostname, std::to_string(port));
    }
    catch(const std::exception &e) {
        log_error().field("message", e.what()).msg("Failed resolving web host. Failing");
        boost::asio::post(mContext, std::bind(handler, boost::system::errc::make_error_code(boost::system::errc::invalid_argument)));
        return;
    }

    boost::asio::async_connect(mSocket.lowest_layer(), endpoints, [this, handler](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
        if (ec) {
            boost::asio::post(mContext, std::bind(handler, ec));
            return;
        }

        mSocket.async_handshake(SocketType::client, [this, handler](boost::system::error_code ec) {
            boost::asio::post(mContext, std::bind(handler, ec));
        });
    });
}

void SSLSocket::asyncWrite(const boost::asio::mutable_buffer &buffer, WriteHandler handler) {
    boost::asio::async_write(mSocket, buffer, handler);
}

void SSLSocket::asyncRead(const boost::asio::mutable_buffer &buffer, ReadHandler handler) {
    boost::asio::async_read(mSocket, buffer, handler);
}

