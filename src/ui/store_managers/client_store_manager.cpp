#include <fstream>

#include "core/log.hpp"
#include "ui/network/network_client.hpp"
#include "ui/store_managers/client_store_manager.hpp"

ClientStoreManager::ClientStoreManager()
    : StoreManager()
    , mNetworkClientState(NetworkClientState::NOT_CONNECTED)
    , mClockDiff(std::chrono::milliseconds(0))
{
    mNetworkClient = std::make_shared<NetworkClient>(getWorkerThread().getContext());

    QObject::connect(mNetworkClient.get(), &NetworkClient::connectionLost, this, &ClientStoreManager::loseConnection);
    QObject::connect(mNetworkClient.get(), &NetworkClient::connectionAttemptFailed, this, &ClientStoreManager::failConnectionAttempt);
    QObject::connect(mNetworkClient.get(), &NetworkClient::connectionShutdown, this, &ClientStoreManager::shutdownConnection);
    QObject::connect(mNetworkClient.get(), &NetworkClient::connectionAttemptSucceeded, this, &ClientStoreManager::succeedConnectionAttempt);
    QObject::connect(mNetworkClient.get(), &NetworkClient::clockSynchronized, this, &ClientStoreManager::synchronizeClock);

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

std::chrono::milliseconds ClientStoreManager::masterTime() const {
    return localTime() + mClockDiff;
}

void ClientStoreManager::synchronizeClock(std::chrono::milliseconds diff) {
    log_debug().field("diff", diff.count()).msg("Synchronize clock");
    mClockDiff = diff;
}

bool ClientStoreManager::canUndo() {
    return false;
}

bool ClientStoreManager::canRedo() {
    return false;
}

void ClientStoreManager::undo() {
    throw std::runtime_error("Attempted to call undo from ClientStoreManager");
}

void ClientStoreManager::undo(ClientActionId action) {
    throw std::runtime_error("Attempted to call undo from ClientStoreManager");
}

void ClientStoreManager::redo() {
    throw std::runtime_error("Attempted to call undo from ClientStoreManager");
}

