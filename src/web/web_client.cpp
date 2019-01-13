#include "log.hpp"
#include "web/web_client.hpp"

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

