#pragma once

#include <QObject>
#include <set>
#include <map>
#include <queue>
#include <boost/asio.hpp> // TODO: Do not include boost convenience headers

#include "network/network_connection.hpp"
#include "network/network_interface.hpp"
#include "network/network_message.hpp"

class NetworkClient : public NetworkInterface {
    Q_OBJECT
public:
    typedef std::list<std::pair<ActionId, std::shared_ptr<Action>>> ActionList;
    typedef std::list<std::pair<ActionId, std::unique_ptr<Action>>> UniqueActionList;

    NetworkClient();

    void postSync(std::unique_ptr<TournamentStore> tournament) override;
    void postAction(ActionId actionId, std::unique_ptr<Action> action) override;
    void postUndo(ActionId actionId) override;

    void postConnect(const std::string &host, unsigned int port);
    void postDisconnect();

    void start() override;
    void quit() override;

    void run() override;

signals:
    void connectionLost();
    void connectionShutdown();
    void connectionAttemptFailed();
    void connectionAttemptSuccess(QTournamentStore *tournament, UniqueActionList *actions, UniqueActionList *unconfirmedActions, std::unordered_set<ActionId> *unconfirmedUndos);
    void syncReceived(QTournamentStore *tournament, UniqueActionList *actions);

private:
    void deliver(std::unique_ptr<NetworkMessage> message);
    void readMessage();
    void writeMessage();
    void killConnection();
    void recoverUnconfirmed(TournamentStore *tournament, ActionList *actions);

    ClientId mId;
    boost::asio::io_context mContext;
    std::optional<boost::asio::ip::tcp::socket> mSocket;
    std::optional<NetworkConnection> mConnection;
    std::string mHost;
    int mPort;
    bool mQuitPosted;

    std::unique_ptr<NetworkMessage> mReadMessage;
    std::queue<std::unique_ptr<NetworkMessage>> mWriteQueue;
    ActionList mUnconfirmedActionList;
    std::unordered_map<ActionId, ActionList::iterator> mUnconfirmedActionMap;
    std::unordered_set<ActionId> mUnconfirmedUndos;
};

