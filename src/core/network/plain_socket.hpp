#pragma once

#include "core/network/network_socket.hpp"

class PlainSocket : public NetworkSocket {
public:
    PlainSocket(boost::asio::io_context &context);
    PlainSocket(boost::asio::io_context &context, boost::asio::ip::tcp::socket mSocket);
    void asyncConnect(boost::asio::ip::tcp::resolver::results_type endpoints, ConnectHandler handler) override;
    void asyncWrite(const boost::asio::mutable_buffer &buffer, WriteHandler handler) override;
    void asyncRead(const boost::asio::mutable_buffer &buffer, ReadHandler handler) override;

private:
    boost::asio::io_context &mContext;
    boost::asio::ip::tcp::socket mSocket;
};

