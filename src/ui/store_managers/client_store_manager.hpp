#pragma once

#include "ui/store_managers/store_manager.hpp"
#include "ui/network/network_client.hpp"

class ClientStoreManager : public StoreManager {
    Q_OBJECT
public:
    ClientStoreManager();

    void connect(QString host, unsigned int port);
    void disconnect();

    NetworkClientState getNetworkClientState() const;
    const NetworkClient& getNetworkClient() const;
    NetworkClient& getNetworkClient();

    std::chrono::milliseconds masterTime() const override;

protected:
    void loseConnection();
    void shutdownConnection();
    void failConnectionAttempt();
    void succeedConnectionAttempt();
    void synchronizeClock(std::chrono::milliseconds diff);

signals:
    void connectionAttemptFailed();
    void connectionLost();
    void connectionShutdown();
    void connectionAttemptSucceeded();
    void networkClientStateChanged(NetworkClientState state);

private:
    std::shared_ptr<NetworkClient> mNetworkClient;
    NetworkClientState mNetworkClientState;
    std::chrono::milliseconds mClockDiff;
};

