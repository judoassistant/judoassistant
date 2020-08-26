#include <boost/asio/bind_executor.hpp>

#include "core/log.hpp"
#include "core/actions/action.hpp"
#include "core/network/network_message.hpp"
#include "core/stores/tournament_store.hpp"
#include "web/tcp_participant.hpp"
#include "web/web_server.hpp"

// TODO: Ensure strands are used correctly
// TODO: Make sure changes are saved every x minutes

TCPParticipant::TCPParticipant(boost::asio::io_context &context, std::shared_ptr<NetworkConnection> connection, WebServer &server, Database &database)
    : mStrand(context)
    , mConnection(std::move(connection))
    , mReadMessage(std::make_unique<NetworkMessage>())
    , mServer(server)
    , mDatabase(database)
    , mState(State::NOT_AUTHENTICATED)
    , mClosePosted(false)
{}

void TCPParticipant::asyncAuth() {
    mReadMessage = std::make_unique<NetworkMessage>();
    auto self = shared_from_this();
    mConnection->asyncRead(*mReadMessage, boost::asio::bind_executor(mStrand, [this, self](boost::system::error_code ec) {
        assert(mState == State::NOT_AUTHENTICATED);

        if (mClosePosted)
            return;

        if (ec) {
            close();
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::REQUEST_WEB_TOKEN) {
            std::string email;
            std::string password;
            if (!mReadMessage->decodeRequestWebToken(email, password)) {
                log_warning().field("message", ec.message()).msg("Encountered error when writing message. Kicking client");
                close();
                return;
            }

            mDatabase.asyncRequestWebToken(email, password, boost::asio::bind_executor(mStrand, [this, self] (WebTokenRequestResponse response, const std::optional<WebToken> token, std::optional<int> userId) {
                assert(mState == State::NOT_AUTHENTICATED);

                if (mClosePosted)
                    return;

                auto message = std::make_unique<NetworkMessage>();
                message->encodeRequestWebTokenResponse(response, token);
                deliver(std::move(message));

                if (response == WebTokenRequestResponse::SUCCESSFUL) {
                    mState = State::AUTHENTICATED;
                    mUserId = userId;
                    asyncTournamentRegister();
                }
                else {
                    asyncAuth();
                }
            }));
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::VALIDATE_WEB_TOKEN) {
            std::string email;
            WebToken token;
            if (!mReadMessage->decodeValidateWebToken(email, token)) {
                log_warning().field("message", ec.message()).msg("Encountered error when writing message. Kicking client");
                close();
                return;
            }

            mDatabase.asyncValidateWebToken(email, token, boost::asio::bind_executor(mStrand, [this, self] (WebTokenValidationResponse response, std::optional<int> userId) {
                assert(mState == State::NOT_AUTHENTICATED);

                if (mClosePosted)
                    return;

                auto message = std::make_unique<NetworkMessage>();
                message->encodeValidateWebTokenResponse(response);
                deliver(std::move(message));

                if (response == WebTokenValidationResponse::SUCCESSFUL) {
                    mState = State::AUTHENTICATED;
                    mUserId = userId;
                    asyncTournamentRegister();
                }
                else {
                    asyncAuth();
                }
            }));
        }
        else {
            log_warning().field("type", mReadMessage->getType()).msg("Received unexpected message type when authenticating");
        }
    }));
}

void TCPParticipant::write() {
    auto self = shared_from_this();
    mConnection->asyncWrite(*(mMessageQueue.front()), boost::asio::bind_executor(mStrand, [this, self](boost::system::error_code ec) {
        if (mClosePosted)
            return;

        if (ec) {
            log_warning().field("message", ec.message()).msg("Encountered error when writing message. Kicking client");
            close();
            return;
        }

        mMessageQueue.pop();
        if (!mMessageQueue.empty())
            write();
    }));
}

void TCPParticipant::deliver(std::shared_ptr<NetworkMessage> message) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, message, self](){
        if (mClosePosted)
            return;

        bool writeInProgress = !mMessageQueue.empty();
        mMessageQueue.push(std::move(message));

        if (!writeInProgress)
            write();
    });
}

void TCPParticipant::asyncTournamentRegister() {
    assert(mState == State::AUTHENTICATED);

    mReadMessage = std::make_unique<NetworkMessage>();
    auto self = shared_from_this();
    mConnection->asyncRead(*mReadMessage, boost::asio::bind_executor(mStrand, [this, self](boost::system::error_code ec) {
        assert(mState == State::AUTHENTICATED);

        if (mClosePosted)
            return;

        if (ec) {
            close();
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::REGISTER_WEB_NAME) {
            TournamentId id;
            std::string webName;
            if (!mReadMessage->decodeRegisterWebName(id, webName)) {
                close();
                return;
            }

            mDatabase.asyncRegisterWebName(*mUserId, id, webName, boost::asio::bind_executor(mStrand, [this, webName, self] (WebNameRegistrationResponse response) {
                assert(mState == State::AUTHENTICATED);

                if (mClosePosted)
                    return;

                auto message = std::make_unique<NetworkMessage>();
                message->encodeRegisterWebNameResponse(response);
                deliver(std::move(message));

                if (response == WebNameRegistrationResponse::SUCCESSFUL) {
                    mState = State::TOURNAMENT_SELECTED;
                    mWebName = std::move(webName);
                    asyncClockSync();
                }
                else {
                    asyncTournamentRegister();
                }
            }));
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::CHECK_WEB_NAME) {
            TournamentId id;
            std::string webName;
            if (!mReadMessage->decodeCheckWebName(id, webName)) {
                close();
                return;
            }

            mDatabase.asyncCheckWebName(*mUserId, id, webName, boost::asio::bind_executor(mStrand, [this, self] (WebNameCheckResponse response) {
                assert(mState == State::AUTHENTICATED);

                if (mClosePosted)
                    return;

                auto message = std::make_unique<NetworkMessage>();
                message->encodeCheckWebNameResponse(response);
                deliver(std::move(message));

                asyncTournamentRegister();
            }));
        }
        else {
            log_warning().field("type", mReadMessage->getType()).msg("Received unexpected message type when registering web name");
        }
    }));
}

struct MoveWrapper {
    std::unique_ptr<WebTournamentStore> tournament;
    SharedActionList actionList;
};

void TCPParticipant::asyncClockSync() {
    mReadMessage = std::make_unique<NetworkMessage>();

    auto message = std::make_shared<NetworkMessage>();
    message->encodeClockSyncRequest();

    auto self = shared_from_this();
    auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    deliver(std::move(message));

    mConnection->asyncRead(*mReadMessage, boost::asio::bind_executor(mStrand, [this, self, t1](boost::system::error_code ec) {
        if (mClosePosted)
            return;

        auto t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        if (mReadMessage->getType() != NetworkMessage::Type::CLOCK_SYNC) {
            close();
            return;
        }

        std::chrono::milliseconds p1;
        if (!mReadMessage->decodeClockSync(p1)) {
            close();
            return;
        }

        mClockDiff = p1 - (t2 + t1)/2;

        mState = State::CLOCK_SYNCED;
        asyncTournamentSync();
    }));
}

void TCPParticipant::asyncTournamentSync() {
    mReadMessage = std::make_unique<NetworkMessage>();
    auto self = shared_from_this();
    mConnection->asyncRead(*mReadMessage, boost::asio::bind_executor(mStrand, [this, self](boost::system::error_code ec) {
        assert(mState == State::CLOCK_SYNCED);

        if (mClosePosted)
            return;

        if (ec) {
            close();
            return;
        }

        if (mReadMessage->getType() != NetworkMessage::Type::SYNC) {
            close();
            return;
        }

        auto wrapper = std::make_shared<MoveWrapper>();
        wrapper->tournament = std::make_unique<WebTournamentStore>();

        if (!mReadMessage->decodeSync(*(wrapper->tournament), wrapper->actionList)) {
            close();
            return;
        }

        // redo all the actions
        auto &tournament = *(wrapper->tournament);
        for (auto &p : wrapper->actionList) {
            auto &action = *(p.second);
            action.redo(tournament);
        }

        mServer.acquireTournament(mWebName, boost::asio::bind_executor(mStrand, [this, wrapper, self](std::shared_ptr<LoadedTournament> loadedTournament) {
            if (mClosePosted)
                return;

            mTournament = std::move(loadedTournament);
            mTournament->setOwner(self);
            mTournament->sync(std::move(wrapper->tournament), std::move(wrapper->actionList), mClockDiff, boost::asio::bind_executor(mStrand, [this, self](bool success) {
                if (mClosePosted)
                    return;

                if (!success) {
                    close();
                    return;
                }

                mDatabase.asyncSetSynced(mWebName, boost::asio::bind_executor(mStrand, [this, self](bool success) {
                    if (mClosePosted)
                        return;

                    if (!success)
                        log_warning().field("webName", mWebName).msg("Failed marking tournament as synced");
                }));

                asyncTournamentListen();
            }));
        }));
    }));
}

void TCPParticipant::asyncTournamentListen() {
    mReadMessage = std::make_unique<NetworkMessage>();
    auto self = shared_from_this();
    mConnection->asyncRead(*mReadMessage, boost::asio::bind_executor(mStrand, [this, self](boost::system::error_code ec) {
        assert(mState == State::CLOCK_SYNCED);

        if (mClosePosted)
            return;

        if (ec) {
            close();
            return;
        }

        auto type = mReadMessage->getType();
        if (type == NetworkMessage::Type::UNDO) {
            ClientActionId actionId;

            if (!mReadMessage->decodeUndo(actionId)) {
                close();
                return;
            }

            mTournament->undo(actionId, [this, self](bool success) {
                if (mClosePosted)
                    return;

                if (success)
                    asyncTournamentListen();
                else
                    close();
            });

            return;
        }

        if (type == NetworkMessage::Type::ACTION) {
            ClientActionId actionId;
            std::shared_ptr<Action> action;

            if (!mReadMessage->decodeAction(actionId, action)) {
                log_warning().msg("Failed decoding action. Kicking client.");
                close();
                return;
            }

            mTournament->dispatch(actionId, std::move(action), boost::asio::bind_executor(mStrand, [this, self](bool success) {
                if (mClosePosted)
                    return;

                if (success)
                    asyncTournamentListen();
                else
                    close();
            }));

            return;
        }

        if (type == NetworkMessage::Type::SYNC) {
            auto tournament = std::make_unique<WebTournamentStore>();
            SharedActionList actionList;

            if (!mReadMessage->decodeSync(*tournament, actionList)) {
                close();
                return;
            }

            mTournament->sync(std::move(tournament), std::move(actionList), mClockDiff, boost::asio::bind_executor(mStrand, [this, self](bool success) {
                if (mClosePosted)
                    return;

                if (success)
                    asyncTournamentListen();
                else
                    close();
            }));

            return;
        }

        asyncTournamentListen();
    }));
}

void TCPParticipant::asyncClose(CloseCallback callback) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, callback]() {
        mClosePosted = true;
        if (mTournament != nullptr)
            mTournament->clearOwner();

        if (mConnection != nullptr)
            mConnection->closeSocket();
        mConnection.reset();
        mServer.leave(shared_from_this(), callback);
    });
}

void TCPParticipant::close() {
    if (mTournament != nullptr)
        mTournament->clearOwner();

    if (mConnection != nullptr)
        mConnection->closeSocket();
    mConnection.reset();
    mServer.leave(shared_from_this(), [](){});
}

