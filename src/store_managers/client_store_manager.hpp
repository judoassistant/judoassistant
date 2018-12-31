#pragma once

#include "store_managers/store_manager.hpp"

class NetworkClient;

class ClientStoreManager : public StoreManager {
    Q_OBJECT
public:
    enum class State {
        LOST_CONNECTION,
        NOT_CONNECTED,
        CONNECTED
    };

    ClientStoreManager();
    ~ClientStoreManager();

    void connect(QString host, unsigned int port);
    void disconnect();

    void dispatch(std::unique_ptr<Action> action) override;
    void undo() override;
    void undo(ClientActionId action) override;
    void redo() override;
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

private:
    void startClient();
    void stopClient();

    State mState;
    std::shared_ptr<NetworkClient> mInterface;
};

