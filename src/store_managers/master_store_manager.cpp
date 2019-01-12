#include <fstream>

#include <boost/system/system_error.hpp>
#include "lz4.h"

#include "log.hpp"
#include "network/network_server.hpp"
#include "serializables.hpp"
#include "store_managers/master_store_manager.hpp"
#include "stores/qtournament_store.hpp"

MasterStoreManager::MasterStoreManager()
    : mDirty(false)
{
    auto &tatamis = getTournament().getTatamis();
    auto location = tatamis.generateLocation(0);
    tatamis[location.handle];
}

MasterStoreManager::~MasterStoreManager() {
    stopServer();
}

bool MasterStoreManager::read(const QString &path) {
    std::ifstream file(path.toStdString(), std::ios::in | std::ios::binary);

    if (!file.is_open())
        return false;

    int compressedSize, uncompressedSize;
    try {
        file.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));
        file.read(reinterpret_cast<char*>(&uncompressedSize), sizeof(uncompressedSize));
    }
    catch (const std::exception &e) {
        return false;
    }

    auto compressed = std::make_unique<char[]>(compressedSize);
    std::string uncompressed;
    uncompressed.resize(uncompressedSize);

    try {
        file.read(compressed.get(), compressedSize);
        file.close();
    }
    catch (const std::exception &e) {
        return false;
    }

    auto returnCode = LZ4_decompress_safe(compressed.get(), uncompressed.data(), compressedSize, uncompressedSize);

    if (returnCode <= 0) {
        log_debug().field("return_value", returnCode).msg("LZ4 decompress failed");
        return false;
    }

    std::istringstream stream(uncompressed);
    auto tournament = std::make_unique<QTournamentStore>();
    try {
        cereal::PortableBinaryInputArchive archive(stream);
        archive(*tournament);
    }
    catch(const std::exception &e) {
        return false;
    }

    log_debug().field("compressedSize", compressedSize).field("uncompressedSize", uncompressedSize).field("path", path.toStdString()).msg("Read tournament from file");

    sync(std::move(tournament));

    mDirty = false;
    return true;
}

void MasterStoreManager::resetTournament() {
    log_debug().msg("Resetting tournament");
    auto tournament = std::make_unique<QTournamentStore>();
    auto &tatamis = getTournament().getTatamis();
    auto location = tatamis.generateLocation(0);
    tatamis[location.handle];
    sync(std::move(tournament));
    mDirty = false;
}

bool MasterStoreManager::write(const QString &path) {
    std::ofstream file(path.toStdString(), std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open())
        return false;

    std::ostringstream stream;

    try {
        cereal::PortableBinaryOutputArchive archive(stream);
        archive(getTournament());
    }
    catch(const std::exception &e) {
        return false;
    }

    const std::string uncompressed = stream.str();
    const int uncompressedSize = static_cast<int>(uncompressed.size());
    const int compressBound = LZ4_compressBound(uncompressedSize);

    auto compressed = std::make_unique<char[]>(compressBound);
    const int compressedSize = LZ4_compress_default(uncompressed.data(), compressed.get(), uncompressed.size(), compressBound);

    if (compressedSize <= 0) {
        log_debug().field("return_value", compressedSize).msg("LZ4 compress failed");
        return false;
    }

    log_debug().field("compressedSize", compressedSize).field("uncompressedSize", uncompressedSize).msg("Done compressing");

    try {
        // TODO: Make this more portable
        file.write(reinterpret_cast<const char*>(&compressedSize), sizeof(compressedSize));
        file.write(reinterpret_cast<const char*>(&uncompressedSize), sizeof(uncompressedSize));
        file.write(compressed.get(), static_cast<size_t>(compressedSize));
        file.close();
    }
    catch(const std::exception &e) {
        return false;
    }

    mDirty = false;
    log_info().field("path", path.toStdString()).field("size(kb)", compressedSize/1024).msg("Wrote tournament to file");
    return true;
}

void MasterStoreManager::startServer(int port) {
    try {
        startInterface(std::make_unique<NetworkServer>(port));
    }
    catch (const boost::system::system_error &e) {
        if (e.code() == boost::system::errc::address_in_use)
            throw AddressInUseException(port);
        else
            throw e;
    }
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

