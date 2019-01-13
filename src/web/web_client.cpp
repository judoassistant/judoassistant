#include "log.hpp"
#include "config/web.hpp"
#include "web/web_client.hpp"

using boost::asio::ip::tcp;

WebClient::WebClient()
    : mContext()
    , mWorkGuard(boost::asio::make_work_guard(mContext))
    , mQuitPosted(false)
{
    qRegisterMetaType<WebNameStatus>("WebNameStatus");
    qRegisterMetaType<WebClient::Status>("WebClientStatus");

}

void WebClient::validateToken(const QString &token) {

}

void WebClient::loginUser(const QString &email, const QString &password) {
    mContext.post([this]() {
        log_debug().msg("Connecting to web");
        mQuitPosted = false;
        tcp::resolver resolver(mContext);
        tcp::resolver::results_type endpoints;
        try {
            endpoints = resolver.resolve(Config::WEB_HOST, std::to_string(Config::WEB_PORT));
        }
        catch(const std::exception &e) {
            log_error().field("message", e.what()).msg("Encountered resolving web host. Failing");
            emit loginFailed();
            return;
        }

        mSocket = tcp::socket(mContext);

        // TODO: Somehow kill when taking too long
        boost::asio::async_connect(*mSocket, endpoints,
        [this](boost::system::error_code ec, tcp::endpoint)
        {
          if (ec) {
            log_error().field("message", ec.message()).msg("Encountered error when connecting to web host. Killing connection");
            killConnection();
            emit loginFailed();
            return;
          }
          else {
            mConnection = NetworkConnection(std::move(*mSocket));
            mSocket.reset();
            mConnection->asyncJoin([this](boost::system::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Encountered error when connecting. Killing connection");
                    killConnection();
                    emit loginFailed();
                    return;
                }

                // TODO: Send login message

            });
          }
        });
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

