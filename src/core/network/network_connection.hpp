#pragma once

#include <functional>
#include <boost/system/error_code.hpp>

#include "core/core.hpp"

class NetworkMessage;
class NetworkSocket;

class NetworkConnection {
public:
    NetworkConnection(std::unique_ptr<NetworkSocket> socket);

    typedef std::function<void(boost::system::error_code)> JoinHandler;
    void asyncJoin(JoinHandler handler);

    typedef std::function<void(boost::system::error_code)> AcceptHandler;
    void asyncAccept(AcceptHandler handler);

    typedef std::function<void(boost::system::error_code)> ReadHandler;
    void asyncRead(NetworkMessage &message, ReadHandler handler);

    typedef std::function<void(boost::system::error_code)> WriteHandler;
    void asyncWrite(NetworkMessage &message, WriteHandler handler);

private:
    void writeHeader(NetworkMessage &message, WriteHandler handler);
    void writeBody(NetworkMessage &message, WriteHandler handler);
    void readHeader(NetworkMessage &message, ReadHandler handler);
    void readBody(NetworkMessage &message, ReadHandler handler);

private:
    std::unique_ptr<NetworkSocket> mSocket;
};

