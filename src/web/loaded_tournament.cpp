#include <boost/asio/post.hpp>
#include <boost/asio/dispatch.hpp>
#include <lz4.h>
#include <sstream>

#include "core/log.hpp"
#include "core/serializables.hpp"
#include "web/loaded_tournament.hpp"
#include "web/web_participant.hpp"

// TODO: Use newer style boost::asio::post in all code
LoadedTournament::LoadedTournament(const std::string &webName, const boost::filesystem::path &dataDirectory, boost::asio::io_context &context, Database &database)
    : mContext(context)
    , mStrand(context)
    , mDatabase(database)
    , mWebName(webName)
    , mFileInUse(false)
    , mFileLocation(dataDirectory / webName)
{
}

struct MoveWrapper {
    std::unique_ptr<TournamentStore> tournament;
    SharedActionList actionList;
};

void LoadedTournament::sync(std::unique_ptr<TournamentStore> tournament, SharedActionList actionList) {
    auto wrapper = std::make_shared<MoveWrapper>();
    wrapper->tournament = std::move(tournament);
    wrapper->actionList = std::move(actionList);

    boost::asio::post(mStrand, [this, wrapper](){
        mTournament = std::move(wrapper->tournament);
        mActionList = std::move(wrapper->actionList);
        mModificationTime = std::chrono::system_clock::now();
    });
}

void LoadedTournament::dispatch(ClientActionId actionId, std::shared_ptr<Action> action) {
    mStrand.post([this, actionId, action](){
        // TODO: Handle exceptions
        action->redo(*mTournament);
        mActionList.push_back({actionId, std::move(action)});
        mActionIds.insert(actionId);
        mModificationTime = std::chrono::system_clock::now();
    });
}

void LoadedTournament::undo(ClientActionId actionId) {
    mStrand.post([this, actionId](){
        // TODO: Handle exceptions

        auto idIt = mActionIds.find(actionId);
        if (idIt == mActionIds.end()) {
            log_warning().msg("Received invalid in tournament. Ignoring");
            return;
        }

        mModificationTime = std::chrono::system_clock::now();

        mActionIds.erase(idIt);

        auto it = std::prev(mActionList.end());
        while (it != mActionList.begin() && it->first != actionId) {
            it->second->undo(*mTournament);
            std::advance(it, -1);
        }

        if (it->first != actionId) {
            log_warning().msg("Did not find action when undoing.");
            std::advance(it, 1);
        }
        else {
            auto it2 = it;
            it = std::next(it2);

            it2->second->undo(*mTournament);
            mActionList.erase(it2);
        }

        while (it != mActionList.end()) {
            it->second->redo(*mTournament);
            std::advance(it, 1);
        }
    });
}

void LoadedTournament::load(LoadCallback callback) {
    boost::asio::post(mStrand, [this, callback]() {
        if (mFileInUse) {
            log_warning().msg("Cancelled load operation: File already in use");
            boost::asio::dispatch(mContext, std::bind(callback, false));
            return;
        }

        mFileInUse = true;

        std::ifstream file(mFileLocation.string(), std::ios::in | std::ios::binary);

        if (!file.is_open()) {
            boost::asio::dispatch(mContext, std::bind(callback, false));
            mFileInUse = false;
            return;
        }

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
            boost::asio::dispatch(mContext, std::bind(callback, false));
            mFileInUse = false;
            return;
        }

        auto compressed = std::make_unique<char[]>(compressedSize);

        std::string uncompressed;
        uncompressed.resize(uncompressedSize);

        try {
            file.read(compressed.get(), compressedSize);
            file.close();
        }
        catch (const std::exception &e) {
            boost::asio::dispatch(mContext, std::bind(callback, false));
            mFileInUse = false;
            return;
        }

        auto returnCode = LZ4_decompress_safe(compressed.get(), uncompressed.data(), compressedSize, uncompressedSize);

        if (returnCode <= 0) {
            boost::asio::dispatch(mContext, std::bind(callback, false));
            mFileInUse = false;
            return;
        }

        auto tournament = std::make_unique<TournamentStore>();
        try {
            std::istringstream stream(uncompressed);
            cereal::PortableBinaryInputArchive archive(stream);
            archive(*tournament);
        }
        catch(const std::exception &e) {
            boost::asio::dispatch(mContext, std::bind(callback, false));
            mFileInUse = false;
            return;
        }

        mTournament = std::move(tournament);
        mActionIds.clear();
        mActionList.clear();

        // TODO: Notify participants
        mSynchronizationTime = std::chrono::system_clock::now();
        mFileInUse = false;
        boost::asio::dispatch(mContext, std::bind(callback, true));
    });
}

void LoadedTournament::save(SaveCallback callback) {
    boost::asio::post(mStrand, [this, callback](){
        if (mFileInUse) {
            log_warning().msg("Cancelled save operation: File already in use");
            boost::asio::dispatch(mContext, std::bind(callback, false));
            return;
        }

        log_debug().msg("Saving tournament");

        mFileInUse = true;

        auto uncompressed = std::make_shared<std::string>();

        try {
            std::ostringstream stream;
            cereal::PortableBinaryOutputArchive archive(stream);
            archive(*mTournament);
            *uncompressed = stream.str();
        }
        catch(const std::exception &e) {
            log_error().msg("Failed serializing of tournament");
            boost::asio::dispatch(mContext, std::bind(callback, false));
            mFileInUse = false;
            return;
        }

        boost::asio::dispatch(mContext, [this, uncompressed, callback]() {
            // Compress string
            const int uncompressedSize = static_cast<int>(uncompressed->size());
            const int compressBound = LZ4_compressBound(uncompressedSize);

            auto compressed = std::make_unique<char[]>(compressBound);
            const int compressedSize = LZ4_compress_default(uncompressed->data(), compressed.get(), uncompressedSize, compressBound);

            if (compressedSize <= 0) {
                log_error().msg("Failed compressing tournament for disk");
                boost::asio::dispatch(mContext, std::bind(callback, false));
                mFileInUse = false;
                return;
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
                boost::asio::dispatch(mContext, std::bind(callback, false));
                mFileInUse = false;
                return;
            }

            assert(header.size() == FILE_HEADER_SIZE);

            // Write header and data to file
            std::ofstream file(mFileLocation.string(), std::ios::out | std::ios::binary | std::ios::trunc);

            if (!file.is_open()) {
                boost::asio::dispatch(mContext, std::bind(callback, false));
                mFileInUse = false;
                return;
            }

            try {
                file.write(header.data(), header.size());
                file.write(compressed.get(), static_cast<size_t>(compressedSize));
                file.close();
            }
            catch(const std::exception &e) {
                boost::asio::dispatch(mContext, std::bind(callback, false));
                mFileInUse = false;
                return;
            }

            boost::asio::dispatch(mStrand, [this, callback]() {
                mFileInUse = false;
                mSynchronizationTime = std::chrono::system_clock::now();
                mDatabase.asyncSetSaveTime(mWebName, mSynchronizationTime, [this](bool success) {
                    if (!success)
                        log_warning().field("webName", mWebName).msg("Failed updating database save_time column");
                });
                boost::asio::dispatch(mContext, std::bind(callback, true));
            });
        });
    });
}

void LoadedTournament::saveIfNeccesary() {
    boost::asio::dispatch(mStrand, [this](){
        if (mModificationTime > mSynchronizationTime) {
            save([this](bool success) {
                if (!success)
                    log_warning().field("webName", mWebName).msg("Failed saving tournament");
            });
        }
    });
}

