#include <boost/asio/connect.hpp>

#include "core/log.hpp"
#include "ui/constants/web.hpp"
#include "ui/web/web_client.hpp"
#include "ui/network/network_server.hpp"

using boost::asio::ip::tcp;

// TODO: Setup read for server crash
WebClient::WebClient(boost::asio::io_context &context)
    : mContext(context)
    , mState(WebClientState::NOT_CONNECTED)
{
    qRegisterMetaType<WebToken>("WebToken");
    qRegisterMetaType<UserRegistrationResponse>("UserRegistrationResponse");
    qRegisterMetaType<WebTokenRequestResponse>("WebTokenRequestResponse");
    qRegisterMetaType<WebTokenValidationResponse>("WebTokenValidationResponse");
    qRegisterMetaType<WebNameCheckResponse>("WebNameCheckResponse");
    qRegisterMetaType<WebNameRegistrationResponse>("WebNameRegistrationResponse");

    qRegisterMetaType<WebClientState>("WebClientState");
}

void WebClient::validateToken(const QString &token) {

}

void WebClient::createConnection(ConnectionHandler handler) {
    mContext.dispatch([this, handler]() {
        assert(mState == WebClientState::NOT_CONNECTED);

        mDisconnecting = false;
        mState = WebClientState::CONNECTING;
        emit stateChanged(mState);

        tcp::resolver resolver(mContext);
        tcp::resolver::results_type endpoints;

        try {
            endpoints = resolver.resolve(Constants::WEB_HOST, std::to_string(Constants::WEB_PORT));
        }
        catch(const std::exception &e) {
            log_error().field("message", e.what()).msg("Failed resolving web host. Failing");
            handler(boost::system::errc::make_error_code(boost::system::errc::invalid_argument));
            return;
        }

        mSocket = tcp::socket(mContext);

        // TODO: Somehow kill when taking too long
        boost::asio::async_connect(*mSocket, endpoints, [this, handler](boost::system::error_code ec, tcp::endpoint) {
            if (ec) {
                log_error().field("message", ec.message()).msg("Encountered error when connecting to web host. Failing");
                killConnection();
                handler(ec);
                return;
            }

            if (mDisconnecting) {
                killConnection();
                handler(boost::system::errc::make_error_code(boost::system::errc::operation_canceled));
                return;
            }

            mConnection = NetworkConnection(std::move(*mSocket));
            mSocket.reset();
            mConnection->asyncJoin([this, handler](boost::system::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Encountered error handshaking with web host. Killing connection");
                    killConnection();
                    handler(ec);
                    return;
                }

                if (mDisconnecting) {
                    killConnection();
                    handler(boost::system::errc::make_error_code(boost::system::errc::operation_canceled));
                    return;
                }

                handler(ec);
            });
        });
    });
}

void WebClient::loginUser(const QString &email, const QString &password) {
    createConnection([this, email, password](boost::system::error_code ec) {
        if (ec) {
            killConnection();
            emit loginFailed(WebTokenRequestResponse::SERVER_ERROR);
            return;
        }

        if (mDisconnecting) {
            killConnection();
            return;
        }

        auto loginMessage = std::make_shared<NetworkMessage>();
        loginMessage->encodeRequestWebToken(email.toStdString(), password.toStdString());
        mConnection->asyncWrite(*loginMessage, [this, loginMessage](boost::system::error_code ec) {
            if (ec) {
                log_error().field("message", ec.message()).msg("Encountered error writing request token message. Killing connection");
                killConnection();
                emit loginFailed(WebTokenRequestResponse::SERVER_ERROR);
                return;
            }

            if (mDisconnecting) {
                killConnection();
                return;
            }

            auto responseMessage = std::make_shared<NetworkMessage>();
            mConnection->asyncRead(*responseMessage, [this, responseMessage](boost::system::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Encountered error reading request token response. Failing");
                    killConnection();
                    emit loginFailed(WebTokenRequestResponse::SERVER_ERROR);
                    return;
                }

                if (mDisconnecting) {
                    killConnection();
                    return;
                }

                if (responseMessage->getType() != NetworkMessage::Type::REQUEST_WEB_TOKEN_RESPONSE) {
                    log_error().msg("Received response message of wrong type. Failing");
                    killConnection();
                    emit loginFailed(WebTokenRequestResponse::SERVER_ERROR);
                    return;
                }

                WebTokenRequestResponse response;
                std::optional<WebToken> token;
                responseMessage->decodeRequestWebTokenResponse(response, token);

                if (response != WebTokenRequestResponse::SUCCESSFUL) {
                    killConnection();
                    emit loginFailed(response);
                    return;
                }

                mState = WebClientState::CONNECTED;
                emit loginSucceeded(token.value());
                emit stateChanged(mState);
            });
        });
    });
}

void WebClient::registerUser(const QString &email, const QString &password) {
    // TODO: Implement registration
}

void WebClient::stop() {
    disconnect();
}

void WebClient::disconnect() {
    mContext.dispatch([this]() {
        if (mState == WebClientState::NOT_CONNECTED)
            return;

        mDisconnecting = true;

        // Let the client send in progress messages first
        if (mState == WebClientState::CONFIGURED && mWriteQueue.empty()) {
            killConnection();
            return;
        }
    });
}

void WebClient::registerWebName(TournamentId id, const QString &webName) {
    log_debug().field("id", id).field("webName", webName.toStdString()).msg("Registering web name");
    mContext.dispatch([this, id, webName]() {
        assert(mState == WebClientState::CONNECTED);
        mState = WebClientState::CONFIGURING;
        emit stateChanged(mState);

        if (mDisconnecting) {
            killConnection();
            return;
        }

        auto registerMessage = std::make_shared<NetworkMessage>();
        registerMessage->encodeRegisterWebName(id, webName.toStdString());
        mConnection->asyncWrite(*registerMessage, [this, registerMessage, webName](boost::system::error_code ec) {
            if (ec) {
                log_error().field("message", ec.message()).msg("Encountered error writing register message. Killing connection");
                killConnection();
                emit registrationFailed(WebNameRegistrationResponse::SERVER_ERROR);
                return;
            }

            if (mDisconnecting) {
                killConnection();
                return;
            }

            auto responseMessage = std::make_shared<NetworkMessage>();
            mConnection->asyncRead(*responseMessage, [this, responseMessage, webName](boost::system::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Encountered error reading registration response. Failing");
                    killConnection();
                    emit registrationFailed(WebNameRegistrationResponse::SERVER_ERROR);
                    return;
                }

                if (mDisconnecting) {
                    killConnection();
                    return;
                }

                log_debug().field("type", responseMessage->getType()).msg("Got response");

                if (responseMessage->getType() != NetworkMessage::Type::REGISTER_WEB_NAME_RESPONSE) {
                    log_error().msg("Received response message of wrong type. Failing");
                    killConnection();
                    emit registrationFailed(WebNameRegistrationResponse::SERVER_ERROR);
                    return;
                }

                WebNameRegistrationResponse response;
                responseMessage->decodeRegisterWebNameResponse(response);

                if (response != WebNameRegistrationResponse::SUCCESSFUL) {
                    killConnection();
                    emit registrationFailed(response);
                    return;
                }

                emit registrationSucceeded(webName);
                emit stateChanged(mState = WebClientState::CONFIGURED);

                enterClockSync();
            });
        });
    });
}

void WebClient::enterClockSync() {
    auto responseMessage = std::make_shared<NetworkMessage>();
    mConnection->asyncRead(*responseMessage, [this, responseMessage](boost::system::error_code ec) {
        if (mDisconnecting) {
            killConnection();
            return;
        }

        if (ec) {
            log_error().field("message", ec.message()).msg("Encountered error reading web server clock sync request message. Failing");
            killConnection();
            return;
        }

        if (responseMessage->getType() != NetworkMessage::Type::CLOCK_SYNC_REQUEST) {
            log_error().msg("Did not receive clock sync request from web server. Failing");
            killConnection();
            return;
        }

        auto message = std::make_shared<NetworkMessage>();
        auto p1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        message->encodeClockSync(p1);
        deliver(std::move(message));

        enterConfigured();
    });
}

void WebClient::enterConfigured() {
    auto message = std::make_shared<NetworkMessage>();
    message->encodeSync(*mNetworkServer->getTournament(), mNetworkServer->getActionStack());
    deliver(std::move(message));

    auto responseMessage = std::make_shared<NetworkMessage>();
    mConnection->asyncRead(*responseMessage, [this, responseMessage](boost::system::error_code ec) {
        if (mDisconnecting) {
            killConnection();
            return;
        }

        if (ec) {
            log_error().field("message", ec.message()).msg("Encountered error reading web server message. Failing");
            killConnection();
            return;
        }

        if (responseMessage->getType() != NetworkMessage::Type::QUIT) {
            log_error().msg("Received response message of wrong type. Failing");
            killConnection();
            return;
        }

        killConnection();
    });
}

void WebClient::checkWebName(TournamentId id, const QString &webName) {
    // TODO: Implement checking of web names
}

void WebClient::killConnection() {
    mConnection.reset();
    mSocket.reset();
    mState = WebClientState::NOT_CONNECTED;
    mDisconnecting = false;
    emit stateChanged(mState);
    while (!mWriteQueue.empty())
        mWriteQueue.pop();
}

void WebClient::deliver(std::shared_ptr<NetworkMessage> message) {
    if (mState != WebClientState::CONFIGURED || mDisconnecting)
        return;

    bool empty = mWriteQueue.empty();
    mWriteQueue.push(std::move(message));

    if (empty)
        writeMessage();
}

void WebClient::writeMessage() {
    mConnection->asyncWrite(*(mWriteQueue.front()), [this](boost::system::error_code ec) {
        if (ec) {
            log_error().field("message", ec.message()).msg("Encountered error when reading message. Disconnecting");
            killConnection();
            return;
        }

        mWriteQueue.pop();
        if (!mWriteQueue.empty()) {
            writeMessage();
        }
        else if (mDisconnecting) {
            killConnection();
            return;
        }
    });
}

void WebClient::setNetworkServer(std::shared_ptr<NetworkServer> networkServer) {
    mNetworkServer = std::move(networkServer);
}

