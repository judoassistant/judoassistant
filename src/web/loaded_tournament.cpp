#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include <lz4.h>
#include <sstream>

#include "core/log.hpp"
#include "core/serializables.hpp"
#include "web/loaded_tournament.hpp"
#include "web/web_participant.hpp"
#include "web/json_encoder.hpp"

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
    std::unique_ptr<WebTournamentStore> tournament;
    SharedActionList actionList;
};

void LoadedTournament::sync(std::unique_ptr<WebTournamentStore> tournament, SharedActionList actionList, SyncCallback callback) {
    auto wrapper = std::make_shared<MoveWrapper>();
    wrapper->tournament = std::move(tournament);
    wrapper->actionList = std::move(actionList);

    boost::asio::post(mStrand, [this, wrapper, callback](){
        mTournament = std::move(wrapper->tournament);
        mActionList = std::move(wrapper->actionList);
        mModificationTime = std::chrono::system_clock::now();

        deliverSync();

        boost::asio::dispatch(mContext, std::bind(callback, true));
    });
}

void LoadedTournament::dispatch(ClientActionId actionId, std::shared_ptr<Action> action, DispatchCallback callback) {
    mStrand.post([this, actionId, action, callback](){
        try {
            action->redo(*mTournament);
        }
        catch (const std::exception &e) {
            boost::asio::dispatch(mContext, std::bind(callback, false));
            return;
        }

        mActionList.push_back({actionId, std::move(action)});
        mActionIds.insert(actionId);
        mModificationTime = std::chrono::system_clock::now();

        deliverChanges();

        boost::asio::dispatch(mContext, std::bind(callback, true));
    });
}

void LoadedTournament::undo(ClientActionId actionId, UndoCallback callback) {
    mStrand.post([this, actionId, callback](){
        auto idIt = mActionIds.find(actionId);
        if (idIt == mActionIds.end()) {
            boost::asio::dispatch(mContext, std::bind(callback, false));
            return;
        }

        // If the action is not found, the loop will be rolled back before failing
        bool success = true;

        try {
            auto it = std::prev(mActionList.end());
            while (it != mActionList.begin() && it->first != actionId) {
                it->second->undo(*mTournament);
                std::advance(it, -1);
            }

            if (it->first != actionId) {
                success = false;
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
        }
        catch (const std::exception &e) {
            success = false;
        }

        if (!success) {
            boost::asio::dispatch(mContext, std::bind(callback, false));
            return;
        }

        deliverChanges();

        mModificationTime = std::chrono::system_clock::now();
        mActionIds.erase(idIt);

        boost::asio::dispatch(mContext, std::bind(callback, true));
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

        auto tournament = std::make_unique<WebTournamentStore>();
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

void LoadedTournament::setOwner(std::weak_ptr<TCPParticipant> owner) {
    mOwner = owner;
}

void LoadedTournament::clearOwner() {
    mOwner.reset();
}

std::weak_ptr<TCPParticipant> LoadedTournament::getOwner() {
    return mOwner;
}

void LoadedTournament::addParticipant(std::shared_ptr<WebParticipant> participant) {
    boost::asio::dispatch(mStrand, [this, participant](){
        JsonEncoder encoder;
        auto message = encoder.encodeSyncMessage(*mTournament, std::nullopt, std::nullopt);
        participant->deliver(std::move(message));

        mWebParticipants.insert(std::move(participant));
    });
}

void LoadedTournament::eraseParticipant(std::shared_ptr<WebParticipant> participant) {
    boost::asio::dispatch(mStrand, [this, participant](){
        mWebParticipants.erase(participant);
        mPlayerSubscriptions.erase(participant);
        mCategorySubscriptions.erase(participant);
    });
}

void LoadedTournament::subscribeCategory(std::shared_ptr<WebParticipant> participant, CategoryId category) {
    boost::asio::dispatch(mStrand, [this, participant, category](){
        mCategorySubscriptions[participant] = category;
        mPlayerSubscriptions.erase(participant);
        JsonEncoder encoder;
        if (!mTournament->containsCategory(category))
            return;
        auto message = encoder.encodeDetailedCategoryMessage(mTournament->getCategory(category));
        participant->deliver(std::move(message));
    });
}

void LoadedTournament::subscribePlayer(std::shared_ptr<WebParticipant> participant, PlayerId player) {
    boost::asio::dispatch(mStrand, [this, participant, player](){
        mPlayerSubscriptions[participant] = player;
        mCategorySubscriptions.erase(participant);
        JsonEncoder encoder;
        if (!mTournament->containsPlayer(player))
            return;
        auto message = encoder.encodeDetailedPlayerMessage(mTournament->getPlayer(player));
        participant->deliver(std::move(message));
    });
}

void LoadedTournament::deliverChanges() {
    JsonEncoder encoder;
    for (const auto & participant : mWebParticipants) {
        std::optional<CategoryId> category;
        auto categoryIt = mCategorySubscriptions.find(participant);
        if (categoryIt != mCategorySubscriptions.end())
            category = categoryIt->second;

        std::optional<PlayerId> player;
        auto playerIt = mPlayerSubscriptions.find(participant);
        if (playerIt != mPlayerSubscriptions.end())
            player = playerIt->second;

        auto buffer = encoder.encodeChangesMessage(*mTournament, category, player);
        participant->deliver(std::move(buffer));
    }

    mTournament->clearChanges();
}

void LoadedTournament::deliverSync() {
    JsonEncoder encoder;
    for (const auto & participant : mWebParticipants) {
        std::optional<CategoryId> category;
        auto categoryIt = mCategorySubscriptions.find(participant);
        if (categoryIt != mCategorySubscriptions.end())
            category = categoryIt->second;

        std::optional<PlayerId> player;
        auto playerIt = mPlayerSubscriptions.find(participant);
        if (playerIt != mPlayerSubscriptions.end())
            player = playerIt->second;

        auto buffer = encoder.encodeSyncMessage(*mTournament, category, player);
        participant->deliver(std::move(buffer));
    }

    mTournament->clearChanges();
}
