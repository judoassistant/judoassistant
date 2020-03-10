#include <fstream>

#include <boost/system/system_error.hpp>
#include <boost/filesystem.hpp>
#include <zstd.h>
#include <QSettings>

#include "core/constants/compression.hpp"
#include "core/log.hpp"
#include "core/serializables.hpp"
#include "ui/network/network_server.hpp"
#include "ui/store_managers/master_store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"

constexpr size_t FILE_HEADER_SIZE = 17;

MasterStoreManager::MasterStoreManager()
    : StoreManager()
    , mWebClientState(WebClientState::NOT_CONNECTED)
    , mWebClient(*this, getWorkerThread().getContext())
    , mNetworkServerState(NetworkServerState::STOPPED)
    , mDirty(false)
{
    // Create first tatami
    auto &tatamis = getTournament().getTatamis();
    auto location = tatamis.generateLocation(0);
    tatamis[location.handle];

    // Set tournament date to today
    getTournament().setQDate(QDate::currentDate());

    mNetworkServer = std::make_shared<NetworkServer>(getWorkerThread().getContext(), mWebClient);
    mWebClient.setNetworkServer(mNetworkServer);
    connect(mNetworkServer.get(), &NetworkServer::stateChanged, this, &MasterStoreManager::changeNetworkServerState);
    connect(&mWebClient, &WebClient::stateChanged, this, &MasterStoreManager::changeWebClientState);
    setInterface(mNetworkServer);

    mSettings = new QSettings(this);
}

void MasterStoreManager::startServer(int port) {
    if (mNetworkServerState != NetworkServerState::STOPPED) {
        log_warning().msg("Tried to start network server when already started");
        return;
    }

    mNetworkServer->start(port);
}

MasterStoreManager::~MasterStoreManager() {

}

void MasterStoreManager::stop() {
    mWebClient.stop();
    StoreManager::stop();
}

bool MasterStoreManager::read(const QString &path) {
    std::ifstream file(path.toStdString(), std::ios::in | std::ios::binary);

    if (!file.is_open())
        return false;

    size_t compressedSize, uncompressedSize;
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

    const size_t returnCode = ZSTD_decompress(uncompressed.data(), uncompressedSize, compressed.get(), compressedSize);

    if (ZSTD_isError(returnCode)) {
        log_error().field("return_value", returnCode).msg("ZSTD decompress failed");
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
    auto tournament = std::make_unique<QTournamentStore>();
    tournament->setId(TournamentId::generate());
    auto &tatamis = getTournament().getTatamis();
    auto location = tatamis.generateLocation(0);
    tatamis[location.handle];
    sync(std::move(tournament));
    mDirty = false;
}

bool MasterStoreManager::write(const QString &path, unsigned int backupCount) {
    const std::string pathStr = path.toStdString();
    // Move old backups
    const std::string extension = boost::filesystem::extension(pathStr);
    const std::string base = pathStr.substr(0, pathStr.size() - extension.size());
    moveBackup(base, 0, extension, backupCount);

    // Open file
    std::ofstream file(pathStr, std::ios::out | std::ios::binary | std::ios::trunc);

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
    const size_t uncompressedSize = uncompressed.size();
    const size_t compressBound = ZSTD_compressBound(uncompressedSize);

    auto compressed = std::make_unique<char[]>(compressBound);
    const size_t compressedSize = ZSTD_compress(compressed.get(), compressBound, uncompressed.data(), uncompressed.size(), COMPRESSION_LEVEL);

    if (ZSTD_isError(compressedSize)) {
        log_error().field("return_value", compressedSize).msg("ZSTD compress failed");
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

void MasterStoreManager::stopServer() {
    mNetworkServer->stop();
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

void MasterStoreManager::changeNetworkServerState(NetworkServerState state) {
    mNetworkServerState = state;

    if (state == NetworkServerState::STARTED)
        sync();
}

void MasterStoreManager::changeWebClientState(WebClientState state) {
    mWebClientState = state;
}

NetworkServer& MasterStoreManager::getNetworkServer() {
    return *mNetworkServer;
}

const NetworkServer& MasterStoreManager::getNetworkServer() const {
    return *mNetworkServer;
}

NetworkServerState MasterStoreManager::getNetworkServerState() const {
    return mNetworkServerState;
}

WebClientState MasterStoreManager::getWebClientState() const {
    return mWebClientState;
}

std::chrono::milliseconds MasterStoreManager::masterTime() const {
    return localTime();
}

QSettings& MasterStoreManager::getSettings() {
    return *mSettings;
}

const QSettings& MasterStoreManager::getSettings() const {
    return *mSettings;
}

bool MasterStoreManager::moveBackup(const std::string &base, unsigned int n, const std::string &extension, unsigned int backupCount) {
    if (n == backupCount)
        return true;

    std::string name;

    if (n == 0)
        name = base + extension;
    else
        name = base + "_backup-" + std::to_string(n) + extension;

    if (!boost::filesystem::exists(name))
        return true;

    const std::string newName = base + "_backup-" + std::to_string(n+1) + extension;

    moveBackup(base, n+1, extension, backupCount);

    try {
        boost::filesystem::rename(name, newName);
    }
    catch (const std::exception &e) {
        return false;
    }

    return true;
}

