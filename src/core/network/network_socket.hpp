#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

// Superclass for SSL and non-SSL sockets
class NetworkSocket {
public:
    typedef std::function<void(boost::system::error_code ec)> ConnectHandler;

    virtual ~NetworkSocket() = default;
    virtual void asyncConnect(boost::asio::ip::tcp::resolver::results_type endpoints, ConnectHandler handler);
private:

};

class SSLSocket : public NetworkSocket {
public:
private:

};

class PlainSocket : public NetworkSocket {
public:
    PlainSocket(boost::asio::io_context &context);
    PlainSocket(boost::asio::io_context &context, boost::asio::ip::tcp::socket mSocket);
    void asyncConnect(boost::asio::ip::tcp::resolver::results_type endpoints, ConnectHandler handler) override;

private:
    boost::asio::io_context &mContext;
    boost::asio::ip::tcp::socket mSocket;
};

