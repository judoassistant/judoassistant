#include <fstream>
#include "ui/store_managers/client_store_manager.hpp"
#include "ui/network/network_client.hpp"

ClientStoreManager::ClientStoreManager()
    : StoreManager()
    : mState(State::NOT_CONNECTED)
{
    auto networkClient = std::make_unique<NetworkClient>(getWorkerThread.getContext());

    QObject::connect(networkClient.get(), &NetworkClient::connectionLost, this, &ClientStoreManager::loseConnection);
    QObject::connect(networkClient.get(), &NetworkClient::connectionAttemptFailed, this, &ClientStoreManager::failConnectionAttempt);
    QObject::connect(networkClient.get(), &NetworkClient::connectionShutdown, this, &ClientStoreManager::shutdownConnection);
    QObject::connect(networkClient.get(), &NetworkClient::connectionAttemptSucceeded, this, &ClientStoreManager::succeedConnectionAttempt);

    setInterface(std::move(networkClient));
}

void ClientStoreManager::asyncConnect(QString host, unsigned int port) {
    if (mState != State::NOT_CONNECTED) {
        log_warning().msg("Tried to call connect when already connecting");
        return;
    }
    mState = State::CONNECTING;
    emit stateChanged(mState);
    mInterface->postConnect(host.toStdString(), port);
}

void ClientStoreManager::asyncDisconnect() {
    if (mState != State::CONNECTED) {
        log_warning().msg("Tried to call disconnect on non-connected client");
        return;
    }
    mState = State::DISCONNECTING;
    emit stateChanged(mState);
    mInterface->postDisconnect();
}

ClientStoreManager::State ClientStoreManager::getState() const {
    return mState;
}

void ClientStoreManager::loseConnection() {
    mState = State::NOT_CONNECTED;
    emit stateChanged(mState);
    emit connectionLost();
}

void ClientStoreManager::shutdownConnection() {
    mState = State::NOT_CONNECTED;
    emit stateChanged(mState);
    emit connectionShutdown();
}

void ClientStoreManager::failConnectionAttempt() {
    mState = State::NOT_CONNECTED;
    emit stateChanged(mState);
    emit connectionAttemptFailed();
}

void ClientStoreManager::succeedConnectionAttempt() {
    mState = State::CONNECTED;
    emit stateChanged(mState);
    emit connectionAttemptSucceeded();
}

