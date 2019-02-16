#include <boost/asio/post.hpp>
#include <boost/asio/dispatch.hpp>
#include <lz4.h>
#include <sstream>

#include "core/log.hpp"
#include "core/serializables.hpp"
#include "web/loaded_tournament.hpp"
#include "web/web_participant.hpp"

// TODO: Use newer style boost::asio::post in all code
LoadedTournament::LoadedTournament(const std::string &webName, const boost::filesystem::path &dataDirectory, boost::asio::io_context &context)
    : mContext(context)
    , mStrand(context)
    , mWebName(webName)
    , mFileInUse(false)
    , mFileLocation(dataDirectory / webName)
    , mDirty(false)
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
        mDirty = true;
    });
}

void LoadedTournament::dispatch(ClientActionId actionId, std::shared_ptr<Action> action) {
    mStrand.post([this, actionId, action](){
        // TODO: Handle exceptions
        action->redo(*mTournament);
        mActionList.push_back({actionId, std::move(action)});
        mActionIds.insert(actionId);
        mDirty = true;
    });
}

void LoadedTournament::undo(ClientActionId actionId) {
    mStrand.post([this, actionId](){
        mDirty = true;
        // TODO: Handle exceptions

        auto idIt = mActionIds.find(actionId);
        if (idIt == mActionIds.end()) {
            log_warning().msg("Received invalid in tournament. Ignoring");
            return;
        }

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
    boost::asio::post(mStrand, [this, callback](){
        if (mFileInUse) {
            log_warning().msg("Cancelled load operation: File already in use");
            callback(false);
            return;
        }
    });
}

void LoadedTournament::save(SaveCallback callback) {
    boost::asio::post(mStrand, [this, callback](){
        if (mFileInUse) {
            log_warning().msg("Cancelled save operation: File already in use");
            callback(false);
            return;
        }

        auto uncompressed = std::make_shared<std::string>();

        try {
            std::ostringstream stream;
            cereal::PortableBinaryOutputArchive archive(stream);
            archive(*mTournament);
            *uncompressed = stream.str();
        }
        catch(const std::exception &e) {
            log_error().msg("Failed serializing of tournament");
            callback(false);
            return;
        }

        mDirty = false;

        boost::asio::dispatch(mContext, [this, uncompressed, callback]() {
            // Compress string
            const int uncompressedSize = static_cast<int>(uncompressed->size());
            const int compressBound = LZ4_compressBound(uncompressedSize);

            auto compressed = std::make_unique<char[]>(compressBound);
            const int compressedSize = LZ4_compress_default(uncompressed->data(), compressed.get(), uncompressedSize, compressBound);

            if (compressedSize <= 0) {
                log_error().msg("Failed compressing tournament for disk");
                callback(false);
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
                callback(false);
                return;
            }

            assert(header.size() == FILE_HEADER_SIZE);

            // Write header and data to file
            std::ofstream file(mFileLocation.string(), std::ios::out | std::ios::binary | std::ios::trunc);

            if (!file.is_open()) {
                callback(false);
                return;
            }

            try {
                file.write(header.data(), header.size());
                file.write(compressed.get(), static_cast<size_t>(compressedSize));
                file.close();
            }
            catch(const std::exception &e) {
                callback(false);
                return;
            }

            boost::asio::dispatch(mStrand, [this, callback]() {
                mFileInUse = false;
                callback(true);
            });
        });
    });
}

