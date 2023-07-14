#include "core/logger.hpp"
#include "core/network/network_message.hpp"
#include "core/web/web_types.hpp"
#include "web/controllers/tournament_controller.hpp"
#include "web/controllers/tournament_controller_session.hpp"
#include "web/gateways/meta_service_gateway.hpp"
#include "web/handlers/tcp_handler.hpp"
#include "web/handlers/tcp_handler_session.hpp"
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

        // auto wrapper = std::make_shared<MoveWrapper>();
        // wrapper->tournament = std::make_unique<WebTournamentStore>();

        // if (!mReadMessage->decodeSync(*(wrapper->tournament), wrapper->actionList)) {
        //     close();
        //     return;
        // }

        // // redo all the actions
        // auto &tournament = *(wrapper->tournament);
        // for (auto &p : wrapper->actionList) {
        //     auto &action = *(p.second);
        //     action.redo(tournament);
        // }


        //     mTournament = std::move(loadedTournament);
        //     mTournament->sync(std::move(wrapper->tournament), std::move(wrapper->actionList), mClockDiff, boost::asio::bind_executor(mStrand, [this, self](bool success) {
        //         if (mClosePosted)
        //             return;

        //         if (!success) {
        //             close();
        //             return;
        //         }

        //         mDatabase.asyncSetSynced(mWebName, boost::asio::bind_executor(mStrand, [this, self](bool success) {
        //             if (mClosePosted)
        //                 return;

        //             if (!success)
        //                 log_warning().field("webName", mWebName).msg("Failed marking tournament as synced");
        //         }));

        //         asyncTournamentListen();
        //     }));
        // }));
    }));

}

void TCPHandlerSession::handleTournamentListen() {

}

void TCPHandlerSession::queueMessage(std::unique_ptr<NetworkMessage> message) {

}


void TCPHandlerSession::close() {
    mIsClosed = true;
    mConnection->closeSocket();
    mConnection.reset();

    // TODO: Remove from handler

}
