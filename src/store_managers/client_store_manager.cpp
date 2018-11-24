#include <fstream>
#include "store_managers/client_store_manager.hpp"
#include "network/network_server.hpp"

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
        throw std::runtime_error("Tried to dispatch action on NOT_CONNECTED ClientStoreManager");
    StoreManager::dispatch(std::move(action));
}

void ClientStoreManager::undo() {
    if (getState() == State::NOT_CONNECTED)
        throw std::runtime_error("Tried to undo on NOT_CONNECTED ClientStoreManager");
    StoreManager::undo();
}

void ClientStoreManager::redo() {
    if (getState() == State::NOT_CONNECTED)
        throw std::runtime_error("Tried to redo on NOT_CONNECTED ClientStoreManager");
    StoreManager::redo();
}

ClientStoreManager::State ClientStoreManager::getState() const {
    return mState;
}

void ClientStoreManager::connectionLost() {
    mState = State::LOST_CONNECTION;
}

void ClientStoreManager::connectionShutdown() {
    mState = State::NOT_CONNECTED;
}

void ClientStoreManager::connectionAttemptSuccess(QTournamentStore *tournament, UniqueActionList *actions, UniqueActionList *unconfirmedActions, std::unordered_set<ActionId> *unconfirmedUndos) {
    // TODO
}

void ClientStoreManager::syncReceived(QTournamentStore *tournament, UniqueActionList *actions) {
    // TODO
}
