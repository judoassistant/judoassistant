#pragma once

#include <boost/system/detail/error_code.hpp>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <boost/asio/io_context_strand.hpp>

#include "core/actions/action.hpp"
#include "web/web_tournament_store.hpp"

class Logger;
class TCPHandlerSession;
class WebHandlerSession;

// WebHandlerSession represents a stateful tournament session. Tournament
// sessions are created by the controller whenever tournaments are read or
// written by handler sessions.
class TournamentControllerSession : public std::enable_shared_from_this<TournamentControllerSession>{
public:
    TournamentControllerSession(boost::asio::io_context &context, Logger &logger);

    typedef std::function<void ()> SyncTournamentCallback;
    void asyncSyncTournament(std::unique_ptr<WebTournamentStore> tournament, SharedActionList actionList, std::chrono::milliseconds clockDiff, SyncTournamentCallback callback);

    typedef std::function<void ()> DispatchActionCallback;
    void asyncDispatchAction(ClientActionId actionID, std::shared_ptr<Action>, DispatchActionCallback callback);

    typedef std::function<void ()> UndoActionCallback;
    void asyncUndoAction(ClientActionId actionID, UndoActionCallback callback);

    typedef std::function<void ()> UpsertTCPSessionCallback;
    void asyncUpsertTCPSession(std::shared_ptr<TCPHandlerSession> tcpSession, UpsertTCPSessionCallback callback);

    typedef std::function<void ()> AddWebSessionCallback;
    void asyncAddWebSession(std::shared_ptr<WebHandlerSession> webSession, AddWebSessionCallback callback);

    typedef std::function<void ()> EraseWebSessionCallback;
    void asyncEraseWebSession(std::shared_ptr<WebHandlerSession> webSession, EraseWebSessionCallback callback);

    typedef std::function<void (boost::system::error_code)> SubscribeCategoryCallback;
    void asyncSubscribeCategory(std::shared_ptr<WebHandlerSession> webSession, CategoryId categoryID, SubscribeCategoryCallback callback);

    typedef std::function<void (boost::system::error_code)> SubscribePlayerCallback;
    void asyncSubscribePlayer(std::shared_ptr<WebHandlerSession> webSession, PlayerId playerID, SubscribePlayerCallback callback);

    typedef std::function<void (boost::system::error_code)> SubscribeTatamiCallback;
    void asyncSubscribeTatami(std::shared_ptr<WebHandlerSession> webSession, unsigned int tatamiIndex, SubscribeTatamiCallback callback);

private:
    void queueSyncMessages();
    void queueChangeMessages();
    void clearSubscriptions(std::shared_ptr<WebHandlerSession> webSession);

    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Logger &mLogger;

    std::unordered_set<std::shared_ptr<WebHandlerSession>> mWebSessions;
    std::shared_ptr<TCPHandlerSession> mTCPSession;

    std::unordered_map<std::shared_ptr<WebHandlerSession>, PlayerId> mPlayerSubscriptions;
    std::unordered_map<std::shared_ptr<WebHandlerSession>, CategoryId> mCategorySubscriptions;
    std::unordered_map<std::shared_ptr<WebHandlerSession>, unsigned int> mTatamiSubscriptions;

    std::unique_ptr<WebTournamentStore> mTournament;
    SharedActionList mActionList;
    std::unordered_set<ClientActionId> mActionIds; // TODO: Is this used anywhere?
    std::chrono::milliseconds mClockDiff;
};
