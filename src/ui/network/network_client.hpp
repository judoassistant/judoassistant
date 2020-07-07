#pragma once

#include <chrono>
#include <map>
#include <queue>
#include <set>

#include <QObject>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "core/network/network_connection.hpp"
#include "core/network/network_message.hpp"
#include "core/network/network_socket.hpp"
#include "ui/network/network_interface.hpp"

enum class NetworkClientState {
    NOT_CONNECTED,
    CONNECTED,
    CONNECTING,
    DISCONNECTING
};

class NetworkClient : public NetworkInterface {
    Q_OBJECT
public:
    NetworkClient(boost::asio::io_context &context);

    void connect(const std::string &host, unsigned int port);
    void disconnect();

    void stop() override;

    void postSync(std::unique_ptr<TournamentStore> tournament) override;
    void postAction(ClientActionId actionId, std::unique_ptr<Action> action) override;
    void postUndo(ClientActionId actionId) override;

signals:
    void connectionAttemptFailed();
    void connectionLost();
    void connectionShutdown();
    void connectionAttemptSucceeded();
    void stateChanged(NetworkClientState state);
    void clockSynchronized(std::chrono::milliseconds diff);

private:
    // different stages of connection
    void connectJoin();
    void connectSynchronizeClocks();
    void connectSync();
    void connectIdle();

    // helper methods
    void deliver(std::unique_ptr<NetworkMessage> message);
    void writeMessage();
    void killConnection();

    NetworkClientState mState;
    boost::asio::io_context &mContext;
    std::unique_ptr<NetworkSocket> mSocket;
    std::optional<NetworkConnection> mConnection;
    std::string mHost;
    int mPort;
    bool mQuitPosted;

    std::unique_ptr<NetworkMessage> mReadMessage;
    std::queue<std::unique_ptr<NetworkMessage>> mWriteQueue;
    SharedActionList mUnconfirmedActionList;
    std::unordered_map<ClientActionId, SharedActionList::iterator> mUnconfirmedActionMap;
};

Q_DECLARE_METATYPE(NetworkClientState)
Q_DECLARE_METATYPE(std::chrono::milliseconds)

