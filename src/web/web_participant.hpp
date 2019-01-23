#pragma once

#include <queue>

#include "network/network_connection.hpp"

class WebServer;
class NetworkMessage;
class WebServerDatabaseWorker;

// Object passed between web server and web workers encapsulating a client
class WebParticipant : public std::enable_shared_from_this<WebParticipant> {
public:
    enum class State {
        NOT_AUTHENTICATED,
        AUTHENTICATED,
        TOURNAMENT_SELECTED,
    };

    WebParticipant() = delete;
    WebParticipant(const WebParticipant &other) = delete;
    WebParticipant(std::shared_ptr<NetworkConnection> connection, WebServer &server, WebServerDatabaseWorker &databaseWorker);

    void quit();

private:
    void asyncAuth();
    void asyncTournamentRegister();

    void write();
    void deliver(std::shared_ptr<NetworkMessage> message);

    std::shared_ptr<NetworkConnection> mConnection;
    std::unique_ptr<NetworkMessage> mReadMessage;
    std::queue<std::shared_ptr<NetworkMessage>> mMessageQueue;
    WebServer &mServer;
    WebServerDatabaseWorker &mDatabaseWorker;
    State mState;
    std::optional<int> mUserId;
};

