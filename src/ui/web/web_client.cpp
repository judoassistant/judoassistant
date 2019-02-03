#include "core/log.hpp"
#include "ui/constants/web.hpp"
#include "ui/web/web_client.hpp"

using boost::asio::ip::tcp;

WebClient::WebClient()
    : mContext()
    , mWorkGuard(boost::asio::make_work_guard(mContext))
    , mQuitPosted(false)
    , mStatus(Status::NOT_CONNECTED)
{
    qRegisterMetaType<WebToken>("WebToken");
    qRegisterMetaType<UserRegistrationResponse>("UserRegistrationResponse");
    qRegisterMetaType<WebTokenRequestResponse>("WebTokenRequestResponse");
    qRegisterMetaType<WebTokenValidationResponse>("WebTokenValidationResponse");
    qRegisterMetaType<WebNameCheckResponse>("WebNameCheckResponse");
    qRegisterMetaType<WebNameRegistrationResponse>("WebNameRegistrationResponse");

    qRegisterMetaType<Status>("WebClient::Status");
}

void WebClient::validateToken(const QString &token) {

}

void WebClient::createConnection(connectionHandler handler) {
    assert(mStatus == Status::NOT_CONNECTED);
    mStatus = Status::CONNECTING;
    emit statusChanged(mStatus);

    mContext.dispatch([this, handler]() {
        mQuitPosted = false;

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

            mConnection = NetworkConnection(std::move(*mSocket));
            mSocket.reset();
            mConnection->asyncJoin([this, handler](boost::system::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Encountered error handshaking with web host. Killing connection");
                    killConnection();
                    handler(ec);
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

        auto loginMessage = std::make_shared<NetworkMessage>();
        loginMessage->encodeRequestWebToken(email.toStdString(), password.toStdString());
        mConnection->asyncWrite(*loginMessage, [this, loginMessage](boost::system::error_code ec) {
            if (ec) {
                log_error().field("message", ec.message()).msg("Encountered error writing request token message. Killing connection");
                killConnection();
                emit loginFailed(WebTokenRequestResponse::SERVER_ERROR);
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

                mStatus = Status::CONNECTED;
                emit loginSucceeded(token.value());
                emit statusChanged(mStatus);
            });
        });
    });
}

void WebClient::registerUser(const QString &email, const QString &password) {
    // TODO: Implement registration
}

void WebClient::disconnect() {
    // TODO: Implement disconnect
}

void WebClient::registerWebName(TournamentId id, const QString &webName) {
    log_debug().field("id", id).field("webName", webName.toStdString()).msg("Registering web name");
    assert(mStatus == Status::CONNECTED);
    mStatus = Status::CONFIGURING;
    emit statusChanged(mStatus);

    auto registerMessage = std::make_shared<NetworkMessage>();
    registerMessage->encodeRegisterWebName(id, webName.toStdString());
    mConnection->asyncWrite(*registerMessage, [this, registerMessage, webName](boost::system::error_code ec) {
        if (ec) {
            log_error().field("message", ec.message()).msg("Encountered error writing register message. Killing connection");
            killConnection();
            emit registrationFailed(WebNameRegistrationResponse::SERVER_ERROR);
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

            mStatus = Status::CONFIGURED;
            emit registrationSucceeded(webName);
            emit statusChanged(mStatus);
        });
    });
}

void WebClient::checkWebName(TournamentId id, const QString &webName) {
    // TODO: Implement checking of web names
}

void WebClient::run() {
    log_info().msg("WebClient thread started");
    try {
        mContext.run();
    }
    catch (std::exception& e)
    {
        log_error().field("msg", e.what()).msg("WebClient caught exception");
    }
    log_debug().msg("WebClient thread stopped");
}

void WebClient::quit() {
    mQuitPosted = true;
    mWorkGuard.reset();
}

void WebClient::killConnection() {
    mConnection.reset();
    mSocket.reset();
    mStatus = Status::NOT_CONNECTED;
    emit statusChanged(mStatus);
    // while (!mWriteQueue.empty())
    //     mWriteQueue.pop();
}

