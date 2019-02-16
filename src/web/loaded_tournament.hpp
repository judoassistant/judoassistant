#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <boost/filesystem/path.hpp>
#include <chrono>
#include <fstream>

#include "core/actions/action.hpp"
#include "core/stores/tournament_store.hpp"
#include "web/database.hpp"

class WebParticipant;

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

private:
    static constexpr unsigned int FILE_HEADER_SIZE = 9;

    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Database &mDatabase;

    // TODO: Store tournament_id or make webName a key in the database
    std::string mWebName;
    std::unique_ptr<TournamentStore> mTournament;
    SharedActionList mActionList;
    std::unordered_set<ClientActionId> mActionIds;
    std::unordered_set<std::unique_ptr<WebParticipant>> mWebParticipants;

    bool mFileInUse;
    boost::filesystem::path mFileLocation;
    std::chrono::system_clock::time_point mSynchronizationTime; // Time when the tournament was last saved/loaded
    std::chrono::system_clock::time_point mModificationTime; // Time when the tournament was last modified
};

