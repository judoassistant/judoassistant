#include <fstream>
#include "ui/store_managers/client_store_manager.hpp"
#include "ui/network/network_client.hpp"

ClientStoreManager::ClientStoreManager()
    : StoreManager()
    , mNetworkClientState(NetworkClientState::NOT_CONNECTED)
{
    mNetworkClient = std::make_shared<NetworkClient>(getWorkerThread().getContext());

    QObject::connect(mNetworkClient.get(), &NetworkClient::connectionLost, this, &ClientStoreManager::loseConnection);
    QObject::connect(mNetworkClient.get(), &NetworkClient::connectionAttemptFailed, this, &ClientStoreManager::failConnectionAttempt);
    QObject::connect(mNetworkClient.get(), &NetworkClient::connectionShutdown, this, &ClientStoreManager::shutdownConnection);
    QObject::connect(mNetworkClient.get(), &NetworkClient::connectionAttemptSucceeded, this, &ClientStoreManager::succeedConnectionAttempt);

    setInterface(mNetworkClient);
}

void ClientStoreManager::connect(QString host, unsigned int port) {
    if (mNetworkClientState != NetworkClientState::NOT_CONNECTED) {
        log_warning().msg("Tried to call connect when already connecting");
        return;
    }
    mNetworkClientState = NetworkClientState::CONNECTING;
    emit networkClientStateChanged(mNetworkClientState);
    mNetworkClient->connect(host.toStdString(), port);
}

void ClientStoreManager::disconnect() {
    if (mNetworkClientState != NetworkClientState::CONNECTED) {
        log_warning().msg("Tried to call disconnect on non-connected client");
        return;
    }
    mNetworkClientState = NetworkClientState::DISCONNECTING;
    emit networkClientStateChanged(mNetworkClientState);
    mNetworkClient->disconnect();
}

NetworkClientState ClientStoreManager::getNetworkClientState() const {
    return mNetworkClientState;
}

void ClientStoreManager::loseConnection() {
    mNetworkClientState = NetworkClientState::NOT_CONNECTED;
    emit networkClientStateChanged(mNetworkClientState);
    emit connectionLost();
}

void ClientStoreManager::shutdownConnection() {
    mNetworkClientState = NetworkClientState::NOT_CONNECTED;
    emit networkClientStateChanged(mNetworkClientState);
    emit connectionShutdown();
}

void ClientStoreManager::failConnectionAttempt() {
    mNetworkClientState = NetworkClientState::NOT_CONNECTED;
    emit networkClientStateChanged(mNetworkClientState);
    emit connectionAttemptFailed();
}

void ClientStoreManager::succeedConnectionAttempt() {
    mNetworkClientState = NetworkClientState::CONNECTED;
    emit networkClientStateChanged(mNetworkClientState);
    emit connectionAttemptSucceeded();
}

const NetworkClient& ClientStoreManager::getNetworkClient() const {
    return *mNetworkClient;
}

NetworkClient& ClientStoreManager::getNetworkClient() {
    return *mNetworkClient;
}

