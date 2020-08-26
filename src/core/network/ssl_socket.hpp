#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

#include "core/network/network_socket.hpp"

class SSLSocket : public NetworkSocket {
public:
    SSLSocket(boost::asio::io_context &context);
    SSLSocket(boost::asio::io_context &context, boost::asio::ip::tcp::socket mSocket);
    void asyncConnect(const std::string &hostname, unsigned int port, ConnectHandler handler) override;
    void asyncWrite(const boost::asio::mutable_buffer &buffer, WriteHandler handler) override;
    void asyncRead(const boost::asio::mutable_buffer &buffer, ReadHandler handler) override;
    void close() override;

private:
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> SocketType;

    boost::asio::io_context &mContext;
    boost::asio::ssl::context mSSLContext;
    SocketType mSocket;
};

