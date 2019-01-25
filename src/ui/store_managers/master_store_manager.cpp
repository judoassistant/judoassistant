#include <fstream>

#include <boost/system/system_error.hpp>
#include <lz4.h>

#include "core/log.hpp"
#include "ui/network/network_server.hpp"
#include "core/serializables.hpp"
#include "ui/store_managers/master_store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"

constexpr size_t FILE_HEADER_SIZE = 9;

MasterStoreManager::MasterStoreManager()
    : mDirty(false)
{
    auto &tatamis = getTournament().getTatamis();
    auto location = tatamis.generateLocation(0);
    tatamis[location.handle];

    mWebClient.start();
}

MasterStoreManager::~MasterStoreManager() {
    stopServer();
    mWebClient.quit();
    mWebClient.wait();
}

bool MasterStoreManager::read(const QString &path) {
    std::ifstream file(path.toStdString(), std::ios::in | std::ios::binary);

    if (!file.is_open())
        return false;

    int compressedSize, uncompressedSize;
    std::string header;
    header.resize(FILE_HEADER_SIZE);
    try {
        file.read(header.data(), FILE_HEADER_SIZE);

        std::istringstream stream(header);
        cereal::PortableBinaryInputArchive archive(stream);
        archive(compressedSize, uncompressedSize);
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
        log_error().field("return_value", returnCode).msg("LZ4 decompress failed");
        return false;
    }

    auto tournament = std::make_unique<QTournamentStore>();
    try {
        std::istringstream stream(uncompressed);
        cereal::PortableBinaryInputArchive archive(stream);
        archive(*tournament);
    }
    catch(const std::exception &e) {
        return false;
    }

    log_info().field("path", path.toStdString()).field("compressedSize(kb)", compressedSize/1000).field("uncompressedSize(kb)", uncompressedSize/1000).msg("Read tournament from file");

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
    // Open file
    std::ofstream file(path.toStdString(), std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open())
        return false;

    // Serialize tournament to string
    std::string uncompressed;

    try {
        std::ostringstream stream;
        cereal::PortableBinaryOutputArchive archive(stream);
        archive(getTournament());
        uncompressed = stream.str();
    }
    catch(const std::exception &e) {
        return false;
    }

    // Compress string
    const int uncompressedSize = static_cast<int>(uncompressed.size());
    const int compressBound = LZ4_compressBound(uncompressedSize);

    auto compressed = std::make_unique<char[]>(compressBound);
    const int compressedSize = LZ4_compress_default(uncompressed.data(), compressed.get(), uncompressed.size(), compressBound);

    if (compressedSize <= 0) {
        log_error().field("return_value", compressedSize).msg("LZ4 compress failed");
        return false;
    }

    // Serialize a header containing size information
    std::string header;
    try {
        std::ostringstream stream;
        cereal::PortableBinaryOutputArchive archive(stream);
        archive(compressedSize, uncompressedSize);
        header = stream.str();
    }
    catch(const std::exception &e) {
        return false;
    }
    assert(header.size() == FILE_HEADER_SIZE);

    // Write header and data to file
    try {
        file.write(header.data(), header.size());
        file.write(compressed.get(), static_cast<size_t>(compressedSize));
        file.close();
    }
    catch(const std::exception &e) {
        return false;
    }

    mDirty = false;
    log_info().field("path", path.toStdString()).field("compressedSize(kb)", compressedSize/1000).field("uncompressedSize(kb)", uncompressedSize/1000).msg("Wrote tournament to file");
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

WebClient& MasterStoreManager::getWebClient() {
    return mWebClient;
}

const WebClient& MasterStoreManager::getWebClient() const {
    return mWebClient;
}
