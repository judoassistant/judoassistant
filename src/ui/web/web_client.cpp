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
    qRegisterMetaType<WebNameCheckResponse>("WebNameCheckResponse");
    qRegisterMetaType<WebClient::Status>("WebClientStatus");

}

void WebClient::validateToken(const QString &token) {

}

void WebClient::createConnection(connectionHandler handler) {
    assert(mStatus == Status::NOT_CONNECTED);
    mStatus = Status::CONNECTING;

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
                handler(ec);
                return;
            }

            mConnection = NetworkConnection(std::move(*mSocket));
            mSocket.reset();
            mConnection->asyncJoin([this, handler](boost::system::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Encountered error handshaking with web host. Killing connection");
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
            emit loginFailed();
            return;
        }

        // auto loginMessage = std::make_shared<NetworkMessage>();
        // loginMessage->encodeRequestWebToken(
        // mConnection->asyncWrite

        log_debug().msg("Should send login message");
        emit loginFailed();
        // TODO: Send login message
    });
}

void WebClient::registerUser(const QString &email, const QString &password) {

}

void WebClient::disconnect() {

}

void WebClient::setWebName(TournamentId id, const QString &webName) {

}

void WebClient::testWebName(TournamentId id, const QString &webName) {

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
    // while (!mWriteQueue.empty())
    //     mWriteQueue.pop();
}

