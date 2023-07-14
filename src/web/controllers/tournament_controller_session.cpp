#include "web/controllers/tournament_controller_session.hpp"
#include "core/logger.hpp"

TournamentControllerSession::TournamentControllerSession(boost::asio::io_context &context, Logger &logger)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
{}

void TournamentControllerSession::asyncSyncTournament(std::unique_ptr<WebTournamentStore> tournament, SharedActionList actionList, std::chrono::milliseconds clockDiff, SyncTournamentCallback callback) {
        // // redo all the actions
        // auto &tournament = *(wrapper->tournament);
        // for (auto &p : wrapper->actionList) {
        //     auto &action = *(p.second);
        //     action.redo(tournament);
        // }
}

void TournamentControllerSession::asyncUndoAction(ClientActionId actionId, UndoActionCallback callback) {

}

void TournamentControllerSession::asyncDispatchAction(ClientActionId actionId, std::shared_ptr<Action>, DispatchActionCallback callback) {

}

void TournamentControllerSession::asyncAddWebSession(std::shared_ptr<WebHandlerSession> participant, DispatchActionCallback callback) {

}

void TournamentControllerSession::asyncEraseWebSession(std::shared_ptr<WebHandlerSession> participant, DispatchActionCallback callback) {

}

void TournamentControllerSession::asyncSubscribeCategory(std::shared_ptr<WebHandlerSession> participant, CategoryId category, DispatchActionCallback callback) {

}

void TournamentControllerSession::asyncSubscribePlayer(std::shared_ptr<WebHandlerSession> participant, PlayerId player, DispatchActionCallback callback) {

}

void TournamentControllerSession::asyncSubscribeTatami(std::shared_ptr<WebHandlerSession> participant, unsigned int index, DispatchActionCallback callback) {

}
