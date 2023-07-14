#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <memory>

#include "core/actions/action.hpp"
#include "web/handlers/web_handler.hpp"
#include "web/handlers/web_handler_session.hpp"
#include "web/web_tournament_store.hpp"

class Logger;

// WebHandlerSession represents a stateful tournament session. Tournament
// sessions are created by the controller whenever tournaments are read or
// written by handler sessions.
class TournamentControllerSession : public std::enable_shared_from_this<TournamentControllerSession>{
public:
    TournamentControllerSession(boost::asio::io_context &context, Logger &logger);

    typedef std::function<void ()> SyncTournamentCallback;
    void asyncSyncTournament(std::unique_ptr<WebTournamentStore> tournament, SharedActionList actionList, std::chrono::milliseconds clockDiff, SyncTournamentCallback callback);

    typedef std::function<void ()> UndoActionCallback;
    void asyncUndoAction(ClientActionId actionId, UndoActionCallback callback);

    typedef std::function<void ()> DispatchActionCallback;
    void asyncDispatchAction(ClientActionId actionId, std::shared_ptr<Action>, DispatchActionCallback callback);

    void asyncAddWebSession(std::shared_ptr<WebHandlerSession> participant, DispatchActionCallback callback);
    void asyncEraseWebSession(std::shared_ptr<WebHandlerSession> participant, DispatchActionCallback callback);
    void asyncSubscribeCategory(std::shared_ptr<WebHandlerSession> participant, CategoryId category, DispatchActionCallback callback);
    void asyncSubscribePlayer(std::shared_ptr<WebHandlerSession> participant, PlayerId player, DispatchActionCallback callback);
    void asyncSubscribeTatami(std::shared_ptr<WebHandlerSession> participant, unsigned int index, DispatchActionCallback callback);

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Logger &mLogger;
};
