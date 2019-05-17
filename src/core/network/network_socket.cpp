#include <boost/asio/connect.hpp>
#include <boost/asio/post.hpp>

#include "core/network/network_socket.hpp"

PlainSocket::PlainSocket(boost::asio::io_context &context)
    : mContext(context)
    , mSocket(context)
{}

PlainSocket::PlainSocket(boost::asio::io_context &context, boost::asio::ip::tcp::socket socket)
    : mContext(context)
    , mSocket(std::move(socket))
{}

void PlainSocket::asyncConnect(boost::asio::ip::tcp::resolver::results_type endpoints, ConnectHandler handler) {
    boost::asio::async_connect(mSocket, endpoints, [this, handler](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
        boost::asio::post(mContext, std::bind(handler, ec));
    });
}

