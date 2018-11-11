#include <fstream>
#include "serializables.hpp"
#include "store_managers/master_store_manager.hpp"
#include "network/network_server.hpp"

MasterStoreManager::MasterStoreManager()
    : mDirty(false)
{}

MasterStoreManager::~MasterStoreManager() {

}

bool MasterStoreManager::read(const QString &path) {
    log_debug().field("path", path).msg("Reading tournament from file");
    std::ifstream file(path.toStdString(), std::ios::in | std::ios::binary);

    if (!file.is_open())
        return false;

    auto tournament = std::make_unique<QTournamentStore>();
    cereal::PortableBinaryInputArchive archive(file);
    archive(*tournament);

    sync(std::move(tournament));

    mDirty = false;
    return true;
}

void MasterStoreManager::resetTournament() {
    log_debug().msg("Resetting tournament");
    sync(std::make_unique<QTournamentStore>());
    mDirty = false;
}

bool MasterStoreManager::write(const QString &path) {
    log_debug().field("path", path).msg("Writing tournament to file");
    std::ofstream file(path.toStdString(), std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open())
        return false;

    cereal::PortableBinaryOutputArchive archive(file);
    archive(getTournament());
    mDirty = false;
    return true;
}

void MasterStoreManager::startServer(int port) {
    startInterface(std::make_unique<NetworkServer>(port));
    sync();
}

void MasterStoreManager::stopServer() {
    stopInterface();
}

void MasterStoreManager::dispatch(std::unique_ptr<Action> action) {
    mDirty = true;
    StoreManager::dispatch(std::move(action));
}

void MasterStoreManager::undo() {
    mDirty = true;
    StoreManager::undo();
}

void MasterStoreManager::redo() {
    mDirty = true;
    StoreManager::redo();
}

bool MasterStoreManager::isDirty() const {
    return mDirty;
}

