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
        mLogger.info("Loading tournament");

        const boost::filesystem::path filePath = mConfig.dataDirectory / tournamentID;
        std::ifstream file(filePath.string(), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            mLogger.info("Tournament not found");
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
            mLogger.info("Unable to read tournament file header", LoggerField(e));
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
            mLogger.info("Unable to read tournament file body", LoggerField(e));
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
            boost::asio::post(mContext, std::bind(callback, ec, nullptr, std::chrono::milliseconds(0)));
            return;
        }

        // Decompress bytes
        std::string decompressed;
        decompressed.resize(uncompressedSize);
        const size_t returnCode = ZSTD_decompress(decompressed.data(), uncompressedSize, compressed.get(), compressedSize);
        if (ZSTD_isError(returnCode)) {
            mLogger.info("Unable to decompress tournament file", LoggerField("returnCode", returnCode));
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
            mLogger.info("Unable to deserialize tournament file contents", LoggerField(e));
            const auto ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
            boost::asio::post(mContext, std::bind(callback, ec, nullptr, std::chrono::milliseconds(0)));
            return;
        }

        const auto ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::asio::post(mContext, std::bind(callback, ec, tournament.release(), clockDiff));
    });
}

void StorageGateway::asyncUpsertTournament(const std::string tournamentID, UpsertTournamentCallback callback) {
    boost::asio::post(mStrand, [this, callback]() {
        const auto ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::asio::post(mContext, std::bind(callback, ec));
    });
}
