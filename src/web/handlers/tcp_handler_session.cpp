#include "core/actions/action.hpp"
#include "core/logger.hpp"
#include "core/network/network_message.hpp"
#include "core/web/web_types.hpp"
#include "web/controllers/tournament_controller.hpp"
#include "web/controllers/tournament_controller_session.hpp"
#include "web/gateways/meta_service_gateway.hpp"
#include "web/handlers/tcp_handler.hpp"
#include "web/handlers/tcp_handler_session.hpp"
#include "web/web_tournament_store.hpp"
#include <boost/asio/bind_executor.hpp>
#include <boost/system/detail/error_code.hpp>
#include <memory>
#include <optional>

TCPHandlerSession::TCPHandlerSession(boost::asio::io_context &context, Logger &logger, TCPHandler &tcpHandler, MetaServiceGateway &metaServiceGateway, TournamentController &tournamentController, std::unique_ptr<NetworkConnection> connection)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
    , mTCPHandler(tcpHandler)
    , mMetaServiceGateway(metaServiceGateway)
    , mTournamentController(tournamentController)
    , mConnection(std::move(connection))
    , mState(State::INITIAL)
    , mIsClosed(false)
{}


void TCPHandlerSession::asyncClose() {

}

void TCPHandlerSession::asyncListen() {
    handleAuthentication();
}

void TCPHandlerSession::handleAuthentication() {
    auto message = std::make_shared<NetworkMessage>();
    auto self = shared_from_this();
    mConnection->asyncRead(*message, boost::asio::bind_executor(mStrand, [this, self, message](boost::system::error_code ec) {
        if (mIsClosed) {
            return;
        }
        if (ec) {
            close();
            return;
        }
        if (message->getType() != NetworkMessage::Type::REQUEST_WEB_TOKEN) {
            mLogger.warn("Received unexpected TCP message type while authenticating", LoggerField("messageType", (size_t) message->getType()));
            close();
            return;
        }

        std::string email, password;
        if (!message->decodeRequestWebToken(email , password)) {
            mLogger.warn("Received invalid REQUEST_WEB_TOKEN request");
            close();
            return;
        }

        mMetaServiceGateway.asyncAuthenticateUser(email, password, boost::asio::bind_executor(mStrand, [this, self] (WebTokenRequestResponse resp, std::optional<int> userID) {
            if (mIsClosed) {
                return;
            }
            if (resp == WebTokenRequestResponse::SERVER_ERROR) {
                close();
                return;
            }

            // Deliver response
            auto message = std::make_unique<NetworkMessage>();
            message->encodeRequestWebTokenResponse(resp, std::nullopt);
            queueMessage(std::move(message));

            // Transition state depending on outcome
            if (resp != WebTokenRequestResponse::SUCCESSFUL) {
                handleAuthentication();
                return;
            }

            mState = State::AUTHENTICATED;
            mUserID = userID.value();
            handleTournamentRegistration();
        }));
    }));
}


void TCPHandlerSession::handleTournamentRegistration() {
    auto message = std::make_shared<NetworkMessage>();
    auto self = shared_from_this();
    mConnection->asyncRead(*message, boost::asio::bind_executor(mStrand, [this, self, message](boost::system::error_code ec) {
        if (mIsClosed) {
            return;
        }
        if (ec) {
            close();
            return;
        }
        if (message->getType() != NetworkMessage::Type::REGISTER_WEB_NAME) {
            mLogger.warn("Received unexpected TCP message type while registering tournament name", LoggerField("messageType", (size_t) message->getType()));
            close();
            return;
        }

        TournamentId tournamentID;
        std::string webName;
        if (!message->decodeRegisterWebName(tournamentID, webName)) {
            mLogger.warn("Received invalid REGISTER_WEB_NAME request");
            close();
            return;
        }

        mTournamentController.asyncAcquireTournament(webName, mUserID, [this, self](WebNameRegistrationResponse resp, std::shared_ptr<TournamentControllerSession> tournamentSession) {
            if (mIsClosed) {
                return;
            }
            if (resp == WebNameRegistrationResponse::SERVER_ERROR) {
                close();
                return;
            }

            // Deliver response
            auto message = std::make_unique<NetworkMessage>();
            message->encodeRegisterWebNameResponse(resp);
            queueMessage(std::move(message));

            // Transition state depending on outcome
            if (resp == WebNameRegistrationResponse::SUCCESSFUL) {
                mState = State::TOURNAMENT_REGISTERED;
                mTournamentSession = std::move(tournamentSession);
                handleTournamentClockSync();
                return;
            }

            handleTournamentRegistration();
        });
    }));
}


void TCPHandlerSession::handleTournamentClockSync() {
    auto clockSyncRequestMessage = std::make_unique<NetworkMessage>();
    clockSyncRequestMessage->encodeClockSyncRequest();
    queueMessage(std::move(clockSyncRequestMessage));

    auto message = std::make_shared<NetworkMessage>();
    auto self = shared_from_this();
    const auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    mConnection->asyncRead(*message, boost::asio::bind_executor(mStrand, [this, self, message, t1](boost::system::error_code ec) {
        const auto t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        if (mIsClosed) {
            return;
        }
        if (ec) {
            close();
            return;
        }
        if (message->getType() != NetworkMessage::Type::CLOCK_SYNC) {
            mLogger.warn("Received unexpected TCP message type while syncing clocks", LoggerField("messageType", (size_t) message->getType()));
            close();
            return;
        }

        std::chrono::milliseconds p1;
        if (!message->decodeClockSync(p1)) {
            mLogger.warn("Received invalid CLOCK_SYNC request");
            close();
            return;
        }

        mClockDiff = p1 - (t2 + t1)/2;
        mState = State::TOURNAMENT_SYNCING;
        handleTournamentSync();
    }));
}

void TCPHandlerSession::handleTournamentSync() {
    auto message = std::make_shared<NetworkMessage>();
    auto self = shared_from_this();
    mConnection->asyncRead(*message, boost::asio::bind_executor(mStrand, [this, self, message](boost::system::error_code ec) {
        if (mIsClosed) {
            return;
        }
        if (ec) {
            close();
            return;
        }
        if (message->getType() != NetworkMessage::Type::SYNC) {
            mLogger.warn("Received unexpected TCP message type while syncing tournament", LoggerField("messageType", (size_t) message->getType()));
            close();
            return;
        }

        SharedActionList actionList;
        std::unique_ptr<WebTournamentStore> tournament;
        if (!message->decodeSync(*tournament, actionList)) {
            mLogger.warn("Received invalid SYNC request");
            close();
            return;
        }

        mTournamentSession->asyncSyncTournament(std::move(tournament), std::move(actionList), mClockDiff, [this, self]() {
            if (mIsClosed) {
                return;
            }

            handleTournamentListen();
        });
    }));
}

void TCPHandlerSession::handleTournamentListen() {
    auto message = std::make_shared<NetworkMessage>();
    auto self = shared_from_this();
    mConnection->asyncRead(*message, boost::asio::bind_executor(mStrand, [this, self, message](boost::system::error_code ec) {
        if (mIsClosed)
            return;
        if (ec) {
            close();
            return;
        }

        const auto type = message->getType();
        if (type == NetworkMessage::Type::UNDO) {
            ClientActionId actionId;
            if (!message->decodeUndo(actionId)) {
                mLogger.warn("Received invalid UNDO request");
                close();
                return;
            }

            mTournamentSession->asyncUndoAction(actionId, [this, self]() {
                if (mIsClosed) {
                    return;
                }

                handleTournamentListen();
            });

            return;
        }
        if (type == NetworkMessage::Type::ACTION) {
            ClientActionId actionId;
            std::shared_ptr<Action> action;
            if (!message->decodeAction(actionId, action)) {
                mLogger.warn("Received invalid ACTION request");
                close();
                return;
            }

            // TODO: Bind executors!!!!!
            mTournamentSession->asyncDispatchAction(actionId, std::move(action),  [this, self]() {
                if (mIsClosed) {
                    return;
                }

                handleTournamentListen();
            });

            return;
        }
        if (type == NetworkMessage::Type::SYNC) {
            auto tournament = std::make_unique<WebTournamentStore>();
            SharedActionList actionList;

            if (!message->decodeSync(*tournament, actionList)) {
                mLogger.warn("Received invalid SYNC request");
                close();
                return;
            }

            mTournamentSession->asyncSyncTournament(std::move(tournament), std::move(actionList), mClockDiff, boost::asio::bind_executor(mStrand, [this, self]() {
                if (mIsClosed)
                    return;

                handleTournamentListen();
            }));

            return;
        }

        mLogger.warn("Received unexpected TCP message type while listening tournament", LoggerField("messageType", (size_t) message->getType()));
        return;
    }));
}

void TCPHandlerSession::queueMessage(std::unique_ptr<NetworkMessage> message) {
    const auto queueWasEmpty = mWriteQueue.empty();
    mWriteQueue.push(std::move(message));

    if (!queueWasEmpty) {
        // The queue was non empty so a write was already in progress.
        return;
    }
    writeMessageQueue();
}

void TCPHandlerSession::writeMessageQueue() {
    auto self = shared_from_this();
    mConnection->asyncWrite(*mWriteQueue.front(), [this, self](boost::system::error_code ec) {
        if (mIsClosed) {
            return;
        }
        if (ec) {
            close();
            return;
        }

        mWriteQueue.pop();
        if (!mWriteQueue.empty()) {
            writeMessageQueue();
        }
    });
}

void TCPHandlerSession::close() {
    mIsClosed = true;
    mConnection->closeSocket();
    mConnection.reset();

    // TODO: Remove from handler

}
