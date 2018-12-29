#include <fstream>
#include "store_managers/client_store_manager.hpp"
#include "network/network_client.hpp"

ClientStoreManager::ClientStoreManager()
    : mState(State::NOT_CONNECTED)
{
    startClient();
}

ClientStoreManager::~ClientStoreManager() {
    stopClient();
}

void ClientStoreManager::startClient() {
    mInterface = std::make_shared<NetworkClient>();

    QObject::connect(mInterface.get(), &NetworkClient::connectionLost, this, &ClientStoreManager::loseConnection);
    QObject::connect(mInterface.get(), &NetworkClient::connectionAttemptFailed, this, &ClientStoreManager::failConnectionAttempt);
    QObject::connect(mInterface.get(), &NetworkClient::connectionShutdown, this, &ClientStoreManager::shutdownConnection);
    QObject::connect(mInterface.get(), &NetworkClient::connectionAttemptSucceeded, this, &ClientStoreManager::succeedConnectionAttempt);

    startInterface(mInterface);
}

void ClientStoreManager::stopClient() {
    stopInterface();
    mInterface.reset();
}

void ClientStoreManager::connect(QString host, unsigned int port) {
    mInterface->postConnect(host.toStdString(), port);
}

void ClientStoreManager::dispatch(std::unique_ptr<Action> action) {
    if (getState() == State::NOT_CONNECTED)
        log_debug().msg("Dispatching action on NOT_CONNECTED ClientStoreManager");
    StoreManager::dispatch(std::move(action));
}

void ClientStoreManager::undo() {
    if (getState() == State::NOT_CONNECTED)
        log_debug().msg("Undoing on on NOT_CONNECTED ClientStoreManager");
    StoreManager::undo();
}

void ClientStoreManager::redo() {
    if (getState() == State::NOT_CONNECTED)
        log_debug().msg("Redoing on on NOT_CONNECTED ClientStoreManager");
    StoreManager::redo();
}

ClientStoreManager::State ClientStoreManager::getState() const {
    return mState;
}

void ClientStoreManager::loseConnection() {
    mState = State::LOST_CONNECTION;
    emit connectionLost();
}

void ClientStoreManager::shutdownConnection() {
    mState = State::NOT_CONNECTED;
    emit connectionShutdown();
}

void ClientStoreManager::failConnectionAttempt() {
    emit connectionAttemptFailed();
}

void ClientStoreManager::succeedConnectionAttempt() {
    mState = State::CONNECTED;
    emit connectionAttemptSucceeded();
}

