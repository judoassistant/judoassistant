#pragma once

#include "store_managers/store_manager.hpp"
#include "network/network_client.hpp"

class ClientStoreManager : public StoreManager {
    Q_OBJECT
public:
    enum class State {
        LOST_CONNECTION,
        NOT_CONNECTED,
        CONNECTED,
    };

    ClientStoreManager();
    ~ClientStoreManager();

    void connect(QString host, unsigned int port);
    void disconnect();

    void dispatch(std::unique_ptr<Action> action);
    void undo();
    void redo();
    State getState() const;

protected:
    typedef std::list<std::pair<ActionId, std::shared_ptr<Action>>> ActionList;
    typedef std::list<std::pair<ActionId, std::unique_ptr<Action>>> UniqueActionList;

    void connectionLost();
    void connectionShutdown();
    void connectionAttemptSuccess(QTournamentStore *tournament, UniqueActionList *actions, UniqueActionList *unconfirmedActions, std::unordered_set<ActionId> *unconfirmedUndos);
    void syncReceived(QTournamentStore *tournament, UniqueActionList *actions);
private:
    void startClient();
    void stopClient();

    State mState;
    std::shared_ptr<NetworkClient> mInterface;
};

