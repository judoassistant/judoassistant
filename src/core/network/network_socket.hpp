#pragma once

#include <boost/asio/io_context.hpp>

// Superclass for SSL and non-SSL sockets
class NetworkSocket {
public:
    typedef std::function<void(boost::system::error_code ec)> ConnectHandler;
    typedef std::function<void(boost::system::error_code ec, std::size_t length)> WriteHandler;
    typedef std::function<void(boost::system::error_code ec, std::size_t length)> ReadHandler;

    virtual ~NetworkSocket() = default;
    virtual void asyncConnect(const std::string &hostname, unsigned int port, ConnectHandler handler) = 0;
    virtual void asyncWrite(const boost::asio::mutable_buffer &buffer, WriteHandler handler) = 0;
    virtual void asyncRead(const boost::asio::mutable_buffer &buffer, ReadHandler handler) = 0;
private:

};

