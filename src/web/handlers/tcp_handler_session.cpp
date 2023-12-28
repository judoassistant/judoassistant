#include <boost/asio/bind_executor.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <memory>
#include <optional>

#include "core/actions/action.hpp"
#include "core/logger.hpp"
#include "core/network/network_connection.hpp"
#include "core/network/network_message.hpp"
#include "core/web/web_types.hpp"
#include "web/controllers/tournament_controller.hpp"
#include "web/controllers/tournament_controller_session.hpp"
#include "web/gateways/meta_service_gateway.hpp"
#include "web/handlers/tcp_handler.hpp"
#include "web/handlers/tcp_handler_session.hpp"
#include "web/models/user_meta.hpp"
#include "web/web_tournament_store.hpp"

TCPHandlerSession::TCPHandlerSession(boost::asio::io_context &context, Logger &logger, TCPHandler &tcpHandler, MetaServiceGateway &metaServiceGateway, TournamentController &tournamentController, std::shared_ptr<NetworkConnection> connection)
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

        UserCredentials userCredentials;
        if (!message->decodeRequestWebToken(userCredentials.email, userCredentials.password)) {
            mLogger.warn("Received invalid REQUEST_WEB_TOKEN request");
            close();
            return;
        }

        mMetaServiceGateway.asyncAuthenticateUser(userCredentials, boost::asio::bind_executor(mStrand, [this, self, userCredentials] (std::optional<Error> error, std::shared_ptr<UserMeta> user) {
            if (mIsClosed) {
                return;
            }
            if (error && error->code != ErrorCode::Unauthorized) {
                close();
                return;
            }


            // Deliver response
            auto message = std::make_unique<NetworkMessage>();
            WebToken token; // TODO: Remove token
            for (size_t i = 0; i < 32; ++i) {
                token[i] = 0;
            }
            bool permissionDenied = error && error->code == ErrorCode::Unauthorized;
            auto webTokenResponse = permissionDenied ? WebTokenRequestResponse::SUCCESSFUL : WebTokenRequestResponse::INCORRECT_CREDENTIALS;
            message->encodeRequestWebTokenResponse(webTokenResponse, token);
            queueMessage(std::move(message));

            // Transition state depending on outcome
            if (permissionDenied) {
                handleAuthentication();
                return;
            }

            mState = State::AUTHENTICATED;
            mUserCredentials = userCredentials;
            mUser = *user;
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

        mTournamentController.asyncAcquireTournament(self, webName, mUser, mUserCredentials, boost::asio::bind_executor(mStrand, [this, self](std::optional<Error> error, std::shared_ptr<TournamentControllerSession> tournamentSession) {
            if (mIsClosed) {
                return;
            }
            if (error && error->code != ErrorCode::Unauthorized) {
                close();
                return;
            }

            // Deliver response
            auto message = std::make_unique<NetworkMessage>();
            bool permissionDenied = error && error->code == ErrorCode::Unauthorized;
            message->encodeRegisterWebNameResponse(permissionDenied ? WebNameRegistrationResponse::OCCUPIED_OTHER_USER : WebNameRegistrationResponse::SUCCESSFUL);
            queueMessage(std::move(message));

            // Transition state depending on outcome
            if (!permissionDenied) {
                mState = State::TOURNAMENT_REGISTERED;
                mTournamentSession = std::move(tournamentSession);
                handleTournamentClockSync();
                return;
            }

            handleTournamentRegistration();
        }));
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
        auto tournament = std::make_unique<WebTournamentStore>();
        if (!message->decodeSync(*tournament, actionList)) {
            mLogger.warn("Received invalid SYNC request");
            close();
            return;
        }

        mTournamentSession->asyncSyncTournament(std::move(tournament), std::move(actionList), mClockDiff, boost::asio::bind_executor(mStrand, [this, self]() {
            if (mIsClosed) {
                return;
            }

            handleTournamentListen();
        }));
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

            mTournamentSession->asyncUndoAction(actionId, boost::asio::bind_executor(mStrand, [this, self]() {
                if (mIsClosed) {
                    return;
                }

                handleTournamentListen();
            }));

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

            mTournamentSession->asyncDispatchAction(actionId, std::move(action),  boost::asio::bind_executor(mStrand, [this, self]() {
                if (mIsClosed) {
                    return;
                }

                handleTournamentListen();
            }));

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
    mConnection->asyncWrite(*mWriteQueue.front(), boost::asio::bind_executor(mStrand, [this, self](boost::system::error_code ec) {
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
    }));
}

void TCPHandlerSession::asyncClose() {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self](){
        if (mIsClosed) {
            return;
        }

        close();
    });
}

void TCPHandlerSession::close() {
    mIsClosed = true;
    mConnection.reset();

    // TODO: Remove from handler

}
