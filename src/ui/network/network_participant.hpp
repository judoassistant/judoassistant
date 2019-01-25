#pragma once

#include <queue>

class NetworkConnection;
class NetworkMessage;
class NetworkServer;

class NetworkParticipant : public std::enable_shared_from_this<NetworkParticipant> {
public:
    NetworkParticipant(std::shared_ptr<NetworkConnection> connection, NetworkServer &server);

    void start();
    void deliver(std::shared_ptr<NetworkMessage> message);

    void setIsSyncing(bool value);
    bool isSyncing() const;

private:
    void readMessage();
    void writeMessage();

    std::shared_ptr<NetworkConnection> mConnection;
    NetworkServer &mServer;
    std::queue<std::shared_ptr<NetworkMessage>> mMessageQueue;
    std::unique_ptr<NetworkMessage> mReadMessage;
    bool mIsSyncing;
};

