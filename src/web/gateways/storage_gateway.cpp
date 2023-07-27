#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/errc.hpp>
#include <chrono>
#include <memory>
#include <fstream>
#include <zstd.h>

#include "core/constants/compression.hpp"
#include "core/serializables.hpp"
#include "web/controllers/tournament_controller_session.hpp"
#include "web/gateways/storage_gateway.hpp"
#include "web/web_tournament_store.hpp"

static constexpr size_t FILE_HEADER_SIZE = 17;

StorageGateway::StorageGateway(boost::asio::io_context &context, Logger &logger, const Config &config)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
    , mConfig(config)
{}

// GetTournament returns an existing tournaments from storage.
void StorageGateway::asyncGetTournament(const std::string tournamentID, GetTournamentCallback callback) {
    boost::asio::post(mStrand, [this, callback, tournamentID]() {
        const boost::filesystem::path filePath = mConfig.dataDirectory / tournamentID;
        std::ifstream file(filePath.string(), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            mLogger.info("Tournament not found in storage");
            // Tournament not found
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory);
            boost::asio::post(mContext, std::bind(callback, ec, nullptr, std::chrono::milliseconds(0)));
            return;
        }

        // Read file header
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
            mLogger.error("Unable to read tournament file header", LoggerField("tournamentID", tournamentID), LoggerField(e));
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
            boost::asio::post(mContext, std::bind(callback, ec, nullptr, std::chrono::milliseconds(0)));
            return;
        }

        // Read compressed file
        auto compressed = std::make_unique<char[]>(compressedSize);
        try {
            file.read(compressed.get(), compressedSize);
            file.close();
        }
        catch (const std::exception &e) {
            mLogger.error("Unable to read tournament file body", LoggerField("tournamentID", tournamentID), LoggerField(e));
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
            boost::asio::post(mContext, std::bind(callback, ec, nullptr, std::chrono::milliseconds(0)));
            return;
        }

        // Decompress bytes
        std::string decompressed;
        decompressed.resize(uncompressedSize);
        const size_t returnCode = ZSTD_decompress(decompressed.data(), uncompressedSize, compressed.get(), compressedSize);
        if (ZSTD_isError(returnCode)) {
            mLogger.error("Unable to decompress tournament file", LoggerField("tournamentID", tournamentID), LoggerField("returnCode", returnCode));
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
            boost::asio::post(mContext, std::bind(callback, ec, nullptr, std::chrono::milliseconds(0)));
            return;
        }

        // Deserialize tournament
        std::chrono::milliseconds clockDiff;
        auto tournament = std::make_unique<WebTournamentStore>();
        try {
            std::istringstream stream(decompressed);
            cereal::PortableBinaryInputArchive archive(stream);
            archive(clockDiff, *tournament);
        }
        catch(const std::exception &e) {
            mLogger.error("Unable to deserialize tournament file contents", LoggerField("tournamentID", tournamentID), LoggerField(e));
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
            boost::asio::post(mContext, std::bind(callback, ec, nullptr, std::chrono::milliseconds(0)));
            return;
        }

        mLogger.info("Read tournament from storage");
        const auto ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::asio::post(mContext, std::bind(callback, ec, tournament.release(), clockDiff));
    });
}

void StorageGateway::asyncUpsertTournament(const std::string tournamentID, WebTournamentStore &tournament, UpsertTournamentCallback callback) {
    // Serialize tournament
    std::chrono::milliseconds clockDiff;
    std::shared_ptr<std::string> decompressed;
    try {
        std::ostringstream stream;
        cereal::PortableBinaryOutputArchive archive(stream);
        archive(clockDiff, tournament);
        decompressed = std::make_shared<std::string>(stream.str());
    }
    catch(const std::exception &e) {
        mLogger.error("Unable to serialize tournament file");
        const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
        boost::asio::post(mContext, std::bind(callback, ec));
        return;
    }

    // Write and compress in separate strand
    boost::asio::post(mStrand, [this, tournamentID, decompressed, callback]() {
        // Compress bytes
        const size_t decompressedSize = decompressed->size();
        const size_t compressBound = ZSTD_compressBound(decompressedSize);

        auto compressed = std::make_unique<char[]>(compressBound);
        const size_t compressedSize = ZSTD_compress(compressed.get(), compressBound, decompressed->data(), decompressedSize, COMPRESSION_LEVEL);

        if (ZSTD_isError(compressedSize)) {
            mLogger.error("Unable to compress tournament file");
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
            boost::asio::post(mContext, std::bind(callback, ec));
            return;
        }

        // Serialize file header
        std::string header;
        try {
            std::ostringstream stream;
            cereal::PortableBinaryOutputArchive archive(stream);
            archive(compressedSize, decompressedSize);
            header = stream.str();
        }
        catch(const std::exception &e) {
            mLogger.error("Unable to serialize file header");
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
            boost::asio::post(mContext, std::bind(callback, ec));
            return;
        }

        assert(header.size() == FILE_HEADER_SIZE);

        // Write file
        const boost::filesystem::path filePath = mConfig.dataDirectory / tournamentID;
        std::ofstream file(filePath.string(), std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            mLogger.error("Unable to open tournament file for writing");
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
            boost::asio::post(mContext, std::bind(callback, ec));
            return;
        }

        try {
            file.write(header.data(), header.size());
            file.write(compressed.get(), static_cast<size_t>(compressedSize));
            file.close();
        }
        catch(const std::exception &e) {
            mLogger.error("Unable to write tournament to file", LoggerField("tournamentID", tournamentID), LoggerField(e));
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
            boost::asio::post(mContext, std::bind(callback, ec));
            return;
        }

        mLogger.info("Upserted tournament to storage", LoggerField("tournamentID", tournamentID));
        const auto ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::asio::post(mContext, std::bind(callback, ec));
    });
}
