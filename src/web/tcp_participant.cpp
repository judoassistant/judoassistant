#include "web/tcp_participant.hpp"
#include "web/web_server.hpp"

TCPParticipant::TCPParticipant(boost::asio::io_context &context, std::shared_ptr<NetworkConnection> connection, WebServer &server, Database &database)
    : mStrand(context)
    , mConnection(std::move(connection))
    , mReadMessage(std::make_unique<NetworkMessage>())
    , mServer(server)
    , mDatabase(database)
    , mState(State::NOT_AUTHENTICATED)
{
}

void TCPParticipant::asyncAuth() {
    log_debug().msg("TCPParticiant: Async auth called");
    auto self = shared_from_this();

    mConnection->asyncRead(*mReadMessage, [this, self](boost::system::error_code ec) {
        log_debug().msg("TCPParticiant: Async read handler called");
        assert(mState == State::NOT_AUTHENTICATED);
        if (ec) {
            log_debug().field("message", ec.message()).msg("Encountered error when reading message. Kicking client");
            mServer.leave(shared_from_this());
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::QUIT) {
            mServer.leave(shared_from_this());
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::REQUEST_WEB_TOKEN) {
            std::string email;
            std::string password;
            if (!mReadMessage->decodeRequestWebToken(email, password)) {
                log_warning().field("message", ec.message()).msg("Encountered error when writing message. Kicking client");
                mServer.leave(shared_from_this());
                return;
            }

            mDatabase.asyncRequestWebToken(email, password, [this] (WebTokenRequestResponse response, const std::optional<WebToken> token, std::optional<int> userId) {
                assert(mState == State::NOT_AUTHENTICATED);
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
            });
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::VALIDATE_WEB_TOKEN) {
            std::string email;
            WebToken token;
            if (!mReadMessage->decodeValidateWebToken(email, token)) {
                log_warning().field("message", ec.message()).msg("Encountered error when writing message. Kicking client");
                mServer.leave(shared_from_this());
                return;
            }

            mDatabase.asyncValidateWebToken(email, token, [this] (WebTokenValidationResponse response, std::optional<int> userId) {
                assert(mState == State::NOT_AUTHENTICATED);
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
            });
        }
        else {
            log_warning().field("type", (unsigned int) mReadMessage->getType()).msg("Received unexpected message type when authenticating");
        }

        mReadMessage = std::make_unique<NetworkMessage>();
    });
}

// TODO: See if shared_from_this pattern can be avoided
void TCPParticipant::write() {
    auto self = shared_from_this();

    mConnection->asyncWrite(*(mMessageQueue.front()), boost::asio::bind_executor(mStrand, [this, self](boost::system::error_code ec) {
        if (ec) {
            log_warning().field("message", ec.message()).msg("Encountered error when writing message. Kicking client");
            mServer.leave(shared_from_this());
            return;
        }

        mMessageQueue.pop();
        if (!mMessageQueue.empty())
            write();
    }));
}

void TCPParticipant::deliver(std::shared_ptr<NetworkMessage> message) {
    // TODO: Capture message by move
    boost::asio::post(mStrand, [this, message](){
        bool writeInProgress = !mMessageQueue.empty();
        mMessageQueue.push(std::move(message));

        if (!writeInProgress)
            write();
    });
}

void TCPParticipant::asyncTournamentRegister() {
    auto self = shared_from_this();

    mConnection->asyncRead(*mReadMessage, [this, self](boost::system::error_code ec) {
        assert(mState == State::AUTHENTICATED);
        if (ec) {
            log_debug().field("message", ec.message()).msg("Encountered error when reading message. Kicking client");
            mServer.leave(shared_from_this());
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::QUIT) {
            mServer.leave(shared_from_this());
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::REGISTER_WEB_NAME) {
            TournamentId id;
            std::string webName;
            if (!mReadMessage->decodeRegisterWebName(id, webName)) {
                log_debug().field("message", ec.message()).msg("Encountered error when reading message. Kicking client");
                mServer.leave(shared_from_this());
                return;
            }

            mDatabase.asyncRegisterWebName(*mUserId, id, webName, [this, webName] (WebNameRegistrationResponse response) {
                assert(mState == State::AUTHENTICATED);
                auto message = std::make_unique<NetworkMessage>();
                message->encodeRegisterWebNameResponse(response);
                deliver(std::move(message));

                if (response == WebNameRegistrationResponse::SUCCESSFUL) {
                    mState = State::TOURNAMENT_SELECTED;
                    mServer.assignWebName(shared_from_this(), webName);
                }
                else {
                    asyncTournamentRegister();
                }
            });
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::CHECK_WEB_NAME) {
            TournamentId id;
            std::string webName;
            if (!mReadMessage->decodeCheckWebName(id, webName)) {
                log_debug().field("message", ec.message()).msg("Encountered error when reading message. Kicking client");
                mServer.leave(shared_from_this());
                return;
            }

            mDatabase.asyncCheckWebName(*mUserId, id, webName, [this] (WebNameCheckResponse response) {
                assert(mState == State::AUTHENTICATED);
                auto message = std::make_unique<NetworkMessage>();
                message->encodeCheckWebNameResponse(response);
                deliver(std::move(message));

                asyncTournamentRegister();
            });
        }
        else {
            log_warning().field("type", (unsigned int) mReadMessage->getType()).msg("Received unexpected message type when authenticating");
        }

        mReadMessage = std::make_unique<NetworkMessage>();
    });
}

void TCPParticipant::quit() {
    // TODO: Implement
    log_debug().msg("Quit called");
}

