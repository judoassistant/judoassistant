#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <boost/filesystem/path.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <chrono>
#include <fstream>

#include "core/actions/action.hpp"
#include "core/stores/tournament_store.hpp"
#include "web/database.hpp"

class WebParticipant;
class TCPParticipant;

class LoadedTournament {
public:
    LoadedTournament(const std::string &webName, const boost::filesystem::path &dataDirectory, boost::asio::io_context &context, Database &database);

    void sync(std::unique_ptr<TournamentStore> tournament, SharedActionList actionList);
    void dispatch(ClientActionId actionId, std::shared_ptr<Action> action);
    void undo(ClientActionId actionId);

    typedef std::function<void (bool)> LoadCallback;
    void load(LoadCallback callback);

    typedef std::function<void (bool)> SaveCallback;
    void save(SaveCallback callback);

    void saveIfNeccesary();

    void setOwner(std::weak_ptr<TCPParticipant> owner);
    void clearOwner();
    std::weak_ptr<TCPParticipant> getOwner();

    void addParticipant(std::shared_ptr<WebParticipant> participant);
    void eraseParticipant(std::shared_ptr<WebParticipant> participant);

    // JSON generating methods
    typedef std::function<void (std::shared_ptr<rapidjson::StringBuffer>)> GenerateSyncJsonCallback;
    void generateSyncJson(GenerateSyncJsonCallback callback);

private:
    rapidjson::Value generatePlayerJson(const PlayerStore &player, rapidjson::Document::AllocatorType& allocator);
    rapidjson::Value generateCategoryJson(const CategoryStore &category, rapidjson::Document::AllocatorType& allocator);

    static constexpr unsigned int FILE_HEADER_SIZE = 9;

    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Database &mDatabase;

    // TODO: Store tournament_id or make webName a key in the database
    std::string mWebName;
    std::unique_ptr<TournamentStore> mTournament;
    SharedActionList mActionList;
    std::unordered_set<ClientActionId> mActionIds;
    std::unordered_set<std::shared_ptr<WebParticipant>> mWebParticipants;
    std::weak_ptr<TCPParticipant> mOwner;

    bool mFileInUse;
    boost::filesystem::path mFileLocation;
    std::chrono::system_clock::time_point mSynchronizationTime; // Time when the tournament was last saved/loaded
    std::chrono::system_clock::time_point mModificationTime; // Time when the tournament was last modified
};

