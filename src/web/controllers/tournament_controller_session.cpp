#include <boost/asio/bind_executor.hpp>
#include <boost/asio/post.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>

#include "web/controllers/tournament_controller_session.hpp"
#include "web/gateways/storage_gateway.hpp"
#include "web/handlers/tcp_handler_session.hpp"
#include "web/handlers/web_handler_session.hpp"
#include "core/constants/actions.hpp"
#include "core/id.hpp"
#include "core/logger.hpp"

TournamentControllerSession::TournamentControllerSession(boost::asio::io_context &context, Logger &logger, const Config &config, StorageGateway &storageGateway, const std::string &tournamentID)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
    , mConfig(config)
    , mStorageGateway(storageGateway)
    , mTournamentID(tournamentID)
    , mIsDirty(false)
{}

TournamentControllerSession::TournamentControllerSession(boost::asio::io_context &context, Logger &logger, const Config &config, StorageGateway &storageGateway, const std::string &tournamentID, std::unique_ptr<WebTournamentStore> tournamentStore, std::chrono::milliseconds clockDiff)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
    , mConfig(config)
    , mStorageGateway(storageGateway)
    , mTournamentID(tournamentID)
    , mTournament(std::move(tournamentStore))
    , mClockDiff(clockDiff)
    , mIsDirty(false)
{
    mTournament->flushWebTatamiModels();
    mTournament->clearChanges();
}

void TournamentControllerSession::asyncSyncTournament(std::unique_ptr<WebTournamentStore> tournament, SharedActionList actionList, std::chrono::milliseconds clockDiff, SyncTournamentCallback callback) {
    // TODO: Capture unique ptrs using moveconstructors.
    auto tournamentPtr = tournament.release();
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, tournamentPtr, actionList, clockDiff, callback]() {
        mTournament = std::unique_ptr<WebTournamentStore>(tournamentPtr);
        mActionList = std::move(actionList);
        mClockDiff = clockDiff;
        mIsDirty = true;

        // Apply all the actions to the tournament
        for (auto &p : actionList) {
            auto &action = *(p.second);
            action.redo(*mTournament);
        }

        mTournament->flushWebTatamiModels();
        mTournament->clearChanges();
        queueSyncMessages();

        // Sync to storage and reset timer for periodic storage syncing
        asyncUpsertToStorage();

        boost::asio::post(mContext, callback);
    });
}

void TournamentControllerSession::asyncUpsertToStorage() {
    auto self = shared_from_this();
    mStorageGateway.asyncUpsertTournament(mTournamentID, *mTournament, boost::asio::bind_executor(mStrand, [self, this](std::optional<Error> error) {
        if (error) {
            // Fail-open when unable to upsert to storage
            mLogger.warn("Unable to upsert tournament to storage", LoggerField("tournamentID", mTournamentID), LoggerField("error", error));
            return;
        }

        mIsDirty = false;
    }));

    // TODO: Implement

    // if (mStorageUpsertTimer) {
    //     mStorageUpsertTimer->expires_from_now(boost::posix_time::seconds(mConfig.saveFrequency));
    // } else {
    //     mStorageUpsertTimer = std::make_unique<boost::asio::deadline_timer>(mContext, boost::posix_time::seconds(mConfig.saveFrequency));
    // }


    // auto self = shared_from_this();
    // mStorageUpsertTimer->async_wait(boost::asio::bind_executor(mStrand, [this, self](const boost::system::error_code &ec) {
    //     if (ec)  {
    //         mLogger.warn("Unable to wait for timer", LoggerField(ec));
    //     }

    //     asyncUpsertToStorage();
    // }));
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
        mIsDirty = true;

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

        // Erase from action ids set
        {
            const auto it = mActionIds.find(actionID);
            if (it != mActionIds.end()) {
                mLogger.error("Unable to undo action. Action does not exist");
                // TODO: Kick TCP session
                return;
            }
            mActionIds.erase(it);
            mIsDirty = true;
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
        if (!mTournament->containsCategory(categoryID)) {
            auto error = std::make_optional<Error>(ErrorCode::NotFound, "category does not exist");
            boost::asio::post(mContext, std::bind(callback, error));
            return;
        }

        mCategorySubscriptions[webSession] = categoryID;
        webSession->notifyCategorySubscription(*mTournament, mTournament->getCategory(categoryID), mClockDiff);
        boost::asio::post(mContext, std::bind(callback, std::nullopt));
    });
}

void TournamentControllerSession::asyncSubscribePlayer(std::shared_ptr<WebHandlerSession> webSession, PlayerId playerID, SubscribePlayerCallback callback) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, webSession, playerID, callback](){
        clearSubscriptions(webSession);
        if (!mTournament->containsPlayer(playerID)) {
            auto error = std::make_optional<Error>(ErrorCode::NotFound, "player does not exist");
            boost::asio::post(mContext, std::bind(callback, error));
            return;
        }

        mPlayerSubscriptions[webSession] = playerID;
        webSession->notifyPlayerSubscription(*mTournament, mTournament->getPlayer(playerID), mClockDiff);
        boost::asio::post(mContext, std::bind(callback, std::nullopt));
    });
}

void TournamentControllerSession::asyncSubscribeTatami(std::shared_ptr<WebHandlerSession> webSession, unsigned int tatamiIndex, SubscribeTatamiCallback callback) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, webSession, tatamiIndex, callback](){
        clearSubscriptions(webSession);
        if (tatamiIndex >= mTournament->getTatamis().tatamiCount()) {
            auto error = std::make_optional<Error>(ErrorCode::NotFound, "tatami does not exist");
            boost::asio::post(mContext, std::bind(callback, error));
            return;
        }

        mTatamiSubscriptions[webSession] = tatamiIndex;
        webSession->notifyTatamiSubscription(*mTournament, tatamiIndex, mClockDiff);
        boost::asio::post(mContext, std::bind(callback, std::nullopt));
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

void TournamentControllerSession::asyncClose() {
    // TODO: Close TCP session
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self]() {
        if (mStorageUpsertTimer) {
            mStorageUpsertTimer->cancel();
            // TODO: Implement upsert to storage if dirty
            // if (mIsDirty) {
            //     mLogger.info("Upserting tournament to storage on close");
            //     mStorageGateway.asyncUpsertTournament(mTournamentID, *mTournament, [this, self](boost::system::error_code ec) {
            //         if (ec) {
            //             // Fail-open when unable to upsert to storage
            //             mLogger.warn("Unable to upsert tournament to storage on close", LoggerField("tournamentID", mTournamentID), LoggerField("errorCode", ec));
            //         }
            //     });
            // }
        }
    });
}
