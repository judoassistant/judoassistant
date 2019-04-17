#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <boost/filesystem/path.hpp>
#include <chrono>
#include <fstream>

#include "core/actions/action.hpp"
#include "web/web_tournament_store.hpp"
#include "web/database.hpp"

class WebParticipant;
class TCPParticipant;

class LoadedTournament {
public:
    LoadedTournament(const std::string &webName, const boost::filesystem::path &dataDirectory, boost::asio::io_context &context, Database &database);

    typedef std::function<void (bool)> SyncCallback;
    void sync(std::unique_ptr<WebTournamentStore> tournament, SharedActionList actionList, std::chrono::milliseconds diff, SyncCallback callback);

    typedef std::function<void (bool)> DispatchCallback;
    void dispatch(ClientActionId actionId, std::shared_ptr<Action> action, DispatchCallback callback);

    typedef std::function<void (bool)> UndoCallback;
    void undo(ClientActionId actionId, UndoCallback callback);

    typedef std::function<void (bool)> LoadCallback;
    void load(LoadCallback callback);

    typedef std::function<void (bool)> SaveCallback;
    void save(SaveCallback callback);

    void saveIfNeccesary();

    void setOwner(std::weak_ptr<TCPParticipant> owner);
    void clearOwner();
    std::weak_ptr<TCPParticipant> getOwner();

    void subscribeCategory(std::shared_ptr<WebParticipant> participant, CategoryId category);
    void subscribePlayer(std::shared_ptr<WebParticipant> participant, PlayerId player);

    void addParticipant(std::shared_ptr<WebParticipant> participant);
    void eraseParticipant(std::shared_ptr<WebParticipant> participant);

private:
    static constexpr unsigned int FILE_HEADER_SIZE = 9;

    void deliverChanges();
    void deliverSync();

    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Database &mDatabase;

    // TODO: Store tournament_id or make webName a key in the database
    std::string mWebName;
    std::unique_ptr<WebTournamentStore> mTournament;
    SharedActionList mActionList;
    std::chrono::milliseconds mClockDiff;
    std::unordered_set<ClientActionId> mActionIds;
    std::unordered_set<std::shared_ptr<WebParticipant>> mWebParticipants;
    std::unordered_map<std::shared_ptr<WebParticipant>, PlayerId> mPlayerSubscriptions;
    std::unordered_map<std::shared_ptr<WebParticipant>, CategoryId> mCategorySubscriptions;
    std::weak_ptr<TCPParticipant> mOwner;

    bool mFileInUse;
    boost::filesystem::path mFileLocation;
    std::chrono::system_clock::time_point mSynchronizationTime; // Time when the tournament was last saved/loaded
    std::chrono::system_clock::time_point mModificationTime; // Time when the tournament was last modified
};

