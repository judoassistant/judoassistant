#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <queue>

#include "core/network/network_connection.hpp"

class WebServer;
class NetworkMessage;
class Database;
class LoadedTournament;

// Object passed between web server and web workers encapsulating a client
class TCPParticipant : public std::enable_shared_from_this<TCPParticipant> {
public:
    enum class State {
        NOT_AUTHENTICATED,
        AUTHENTICATED,
        TOURNAMENT_SELECTED,
    };

    TCPParticipant() = delete;
    TCPParticipant(const TCPParticipant &other) = delete;
    TCPParticipant(boost::asio::io_context &context, std::shared_ptr<NetworkConnection> connection, WebServer &server, Database &database);

    void quit();
    void asyncAuth();

private:
    void asyncTournamentRegister();
    void asyncTournamentSync();
    void asyncTournamentListen();

    void write();
    void deliver(std::shared_ptr<NetworkMessage> message);

    boost::asio::io_context::strand mStrand;
    std::shared_ptr<NetworkConnection> mConnection;
    std::unique_ptr<NetworkMessage> mReadMessage;
    std::queue<std::shared_ptr<NetworkMessage>> mMessageQueue;
    WebServer &mServer;
    Database &mDatabase;
    State mState;
    std::optional<int> mUserId;
    std::string mWebName;
    std::shared_ptr<LoadedTournament> mTournament;
};

