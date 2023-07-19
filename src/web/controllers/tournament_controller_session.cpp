#include <boost/asio/post.hpp>
#include <boost/system/detail/errc.hpp>

#include "web/controllers/tournament_controller_session.hpp"
#include "web/handlers/tcp_handler_session.hpp"
#include "web/handlers/web_handler_session.hpp"
#include "core/constants/actions.hpp"
#include "core/id.hpp"
#include "core/logger.hpp"

TournamentControllerSession::TournamentControllerSession(boost::asio::io_context &context, Logger &logger)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
{}

void TournamentControllerSession::asyncSyncTournament(std::unique_ptr<WebTournamentStore> tournament, SharedActionList actionList, std::chrono::milliseconds clockDiff, SyncTournamentCallback callback) {
    // TODO: Capture unique ptrs using move constructors.
    auto tournamentPtr = tournament.release();
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, tournamentPtr, actionList, clockDiff]() {
        mTournament = std::unique_ptr<WebTournamentStore>(tournamentPtr);
        mActionList = std::move(actionList);
        mClockDiff = clockDiff;

        mTournament->flushWebTatamiModels();
        queueSyncMessages();

        // Apply all the actions to the tournament
        for (auto &p : actionList) {
            auto &action = *(p.second);
            action.redo(*mTournament);
        }
    });
}

void TournamentControllerSession::asyncDispatchAction(ClientActionId actionID, std::shared_ptr<Action> action, DispatchActionCallback callback) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, actionID, action, callback]() {
        try {
            action->redo(*mTournament);
        }
        catch (const std::exception &e) {
            mLogger.error("Unable to redo action", LoggerField(e));
            boost::asio::post(mContext, callback);
            return;
        }

        mActionList.emplace_back(actionID, std::move(action));
        mActionIds.insert(actionID);

        if (mActionList.size() > MAX_ACTION_STACK_SIZE) {
            mActionIds.erase(mActionList.front().first);
            mActionList.pop_front();
        }

        mTournament->flushWebTatamiModels();
        queueChangeMessages();
        mTournament->clearChanges();

        boost::asio::post(mContext, callback);
    });
}

void TournamentControllerSession::asyncUndoAction(ClientActionId actionID, UndoActionCallback callback) {
    auto self = shared_from_this();
    boost::asio::post([this, self, actionID, callback] {
        // TODO: Catch exceptions

        // Verify that the action exists
        {
            const auto it = mActionIds.find(actionID);
            if (it != mActionIds.end()) {
                mLogger.error("Unable to undo action. Action does not exist");
                // TODO: Kick TCP session
                return;
            }
            mActionIds.erase(it);
        }

        // Rollback the stack by undoing actions until the action is found
        auto it = std::prev(mActionList.end());
        while (it != mActionList.begin() && it->first != actionID) {
            it->second->undo(*mTournament);
            it = std::prev(it);
        }

        // Undo and remove the action
        auto tempIt = it;
        it = std::next(it);
        tempIt->second->undo(*mTournament);
        mActionList.erase(tempIt);

        // Rollout the stack by redo all actions
        while (it != mActionList.end()) {
            it->second->redo(*mTournament);
            it = std::next(it);
        }

        // Flush changes and call back
        mTournament->flushWebTatamiModels();
        queueChangeMessages();
        mTournament->clearChanges();
        boost::asio::post(mContext, callback);
    });
}


void TournamentControllerSession::asyncAddWebSession(std::shared_ptr<WebHandlerSession> webSession, AddWebSessionCallback callback) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, webSession, callback]() {
        webSession->notifyTournamentSync(*mTournament, std::nullopt, std::nullopt, std::nullopt, mClockDiff);
        mWebSessions.insert(std::move(webSession));
        boost::asio::post(mContext, callback);
    });
}

void TournamentControllerSession::asyncEraseWebSession(std::shared_ptr<WebHandlerSession> webSession, EraseWebSessionCallback callback) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, webSession, callback]() {
        mWebSessions.erase(webSession);
        clearSubscriptions(webSession);
        boost::asio::post(mContext, callback);
    });
}

void TournamentControllerSession::clearSubscriptions(std::shared_ptr<WebHandlerSession> webSession) {
    mCategorySubscriptions.erase(webSession);
    mPlayerSubscriptions.erase(webSession);
    mTatamiSubscriptions.erase(webSession);
}

void TournamentControllerSession::asyncSubscribeCategory(std::shared_ptr<WebHandlerSession> webSession, CategoryId categoryID, SubscribeCategoryCallback callback) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, webSession, categoryID, callback](){
        clearSubscriptions(webSession);
        auto errCode = boost::system::errc::no_such_file_or_directory;
        if (mTournament->containsCategory(categoryID)) {
            mCategorySubscriptions[webSession] = categoryID;
            webSession->notifyCategorySubscription(*mTournament, mTournament->getCategory(categoryID), mClockDiff);
            errCode = boost::system::errc::success;
        }
        boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(errCode)));
    });
}

void TournamentControllerSession::asyncSubscribePlayer(std::shared_ptr<WebHandlerSession> webSession, PlayerId playerID, SubscribePlayerCallback callback) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, webSession, playerID, callback](){
        clearSubscriptions(webSession);
        auto errCode = boost::system::errc::no_such_file_or_directory;
        if (mTournament->containsPlayer(playerID)) {
            mPlayerSubscriptions[webSession] = playerID;
            webSession->notifyPlayerSubscription(*mTournament, mTournament->getPlayer(playerID), mClockDiff);
            errCode = boost::system::errc::success;
        }
        boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(errCode)));
    });
}

void TournamentControllerSession::asyncSubscribeTatami(std::shared_ptr<WebHandlerSession> webSession, unsigned int tatamiIndex, SubscribeTatamiCallback callback) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, webSession, tatamiIndex, callback](){
        clearSubscriptions(webSession);
        auto errCode = boost::system::errc::no_such_file_or_directory;
        if (tatamiIndex < mTournament->getTatamis().tatamiCount()) {
            mTatamiSubscriptions[webSession] = tatamiIndex;
            webSession->notifyTatamiSubscription(*mTournament, tatamiIndex, mClockDiff);
            errCode = boost::system::errc::success;
        }
        boost::asio::post(mContext, std::bind(callback, boost::system::errc::make_error_code(errCode)));
    });
}

void TournamentControllerSession::queueSyncMessages() {
    for (auto &webSession : mWebSessions) {
        std::optional<CategoryId> categoryId;
        if (const auto categoryIt = mCategorySubscriptions.find(webSession); categoryIt != mCategorySubscriptions.end()) {
            categoryId = categoryIt->second;
        }
        std::optional<PlayerId> playerId;
        if (const auto playerIt = mPlayerSubscriptions.find(webSession); playerIt != mPlayerSubscriptions.end()) {
            playerId = playerIt->second;
        }
        std::optional<unsigned int> tatamiIndex;
        if (auto tatamiIt = mTatamiSubscriptions.find(webSession); tatamiIt != mTatamiSubscriptions.end()) {
            tatamiIndex = tatamiIt->second;
        }

        webSession->notifyTournamentSync(*mTournament, categoryId, playerId, tatamiIndex, mClockDiff);
    }
}

void TournamentControllerSession::queueChangeMessages() {
    for (auto &webSession : mWebSessions) {
        std::optional<CategoryId> categoryId;
        if (const auto categoryIt = mCategorySubscriptions.find(webSession); categoryIt != mCategorySubscriptions.end()) {
            categoryId = categoryIt->second;
        }
        std::optional<PlayerId> playerId;
        if (const auto playerIt = mPlayerSubscriptions.find(webSession); playerIt != mPlayerSubscriptions.end()) {
            playerId = playerIt->second;
        }
        std::optional<unsigned int> tatamiIndex;
        if (auto tatamiIt = mTatamiSubscriptions.find(webSession); tatamiIt != mTatamiSubscriptions.end()) {
            tatamiIndex = tatamiIt->second;
        }

        if (!mTournament->isChanged(categoryId, playerId, tatamiIndex)) {
            continue;
        }

        webSession->notifyTournamentChange(*mTournament, categoryId, playerId, tatamiIndex, mClockDiff);
    }

    // TODO: Sync to meta service?
}

void TournamentControllerSession::asyncUpsertTCPSession(std::shared_ptr<TCPHandlerSession> tcpSession, UpsertTCPSessionCallback callback) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, tcpSession, callback](){
        if (mTCPSession) {
            mTCPSession->asyncClose();
        }

        mTCPSession = std::move(tcpSession);
        boost::asio::post(mContext, callback);
    });
}
