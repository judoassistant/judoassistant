#pragma once

#include "ui/store_managers/store_manager.hpp"

class NetworkClient;

class ClientStoreManager : public StoreManager {
    Q_OBJECT
public:
    enum class State {
        NOT_CONNECTED,
        CONNECTED,
        CONNECTING,
        DISCONNECTING
    };

    ClientStoreManager();

    void asyncConnect(QString host, unsigned int port);
    void asyncDisconnect();

    State getState() const;

protected:
    void loseConnection();
    void shutdownConnection();
    void failConnectionAttempt();
    void succeedConnectionAttempt();

signals:
    void connectionAttemptFailed();
    void connectionLost();
    void connectionShutdown();
    void connectionAttemptSucceeded();
    void stateChanged(State state);

private:
    State mState;
    std::shared_ptr<NetworkClient> mInterface;
};

