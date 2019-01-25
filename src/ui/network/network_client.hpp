#pragma once

#include <QObject>
#include <set>
#include <map>
#include <queue>
#include <boost/asio.hpp> // TODO: Do not include boost convenience headers

#include "core/network/network_connection.hpp"
#include "core/network/network_message.hpp"
#include "ui/network/network_interface.hpp"
#include "ui/store_managers/sync_payload.hpp"

class NetworkClient : public NetworkInterface {
    Q_OBJECT
public:
    NetworkClient();

    void postSync(std::unique_ptr<TournamentStore> tournament) override;
    void postAction(ClientActionId actionId, std::unique_ptr<Action> action) override;
    void postUndo(ClientActionId actionId) override;

    void postConnect(const std::string &host, unsigned int port);
    void postDisconnect();

    void start() override;
    void quit() override;

    void run() override;

signals:
    void connectionAttemptFailed();
    void connectionLost();
    void connectionShutdown();
    void connectionAttemptSucceeded();

private:
    void deliver(std::unique_ptr<NetworkMessage> message);
    void readMessage();
    void writeMessage();
    void killConnection();
    void recoverUnconfirmed(TournamentStore *tournament, SharedActionList *actions);

    boost::asio::io_context mContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;
    std::optional<boost::asio::ip::tcp::socket> mSocket;
    std::optional<NetworkConnection> mConnection;
    std::string mHost;
    int mPort;
    bool mQuitPosted;

    std::unique_ptr<NetworkMessage> mReadMessage;
    std::queue<std::unique_ptr<NetworkMessage>> mWriteQueue;
    SharedActionList mUnconfirmedActionList;
    std::unordered_map<ClientActionId, SharedActionList::iterator> mUnconfirmedActionMap;
    std::unordered_set<ClientActionId> mUnconfirmedUndos;
};

