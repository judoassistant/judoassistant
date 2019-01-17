#include "log.hpp"
#include "web/web_server_database_worker.hpp"
#include "config/web_server.hpp"
#include "botan-2/botan/rng.h"
#include "botan-2/botan/system_rng.h"
#include "botan-2/botan/bcrypt.h"

WebServerDatabaseWorker::WebServerDatabaseWorker(boost::asio::io_context &masterContext)
    : mContext()
    , mMasterContext(masterContext)
    , mWorkGuard(boost::asio::make_work_guard(mContext))
    , mConnection(Config::POSTGRES_CONFIG)
{

}

void WebServerDatabaseWorker::run() {
    mContext.run();
}

void WebServerDatabaseWorker::quit() {
    mWorkGuard.reset();
}

void WebServerDatabaseWorker::asyncRequestToken(const std::string &email, const std::string &password, tokenRequestCallback callback) {
    boost::asio::post(mContext, std::bind(&WebServerDatabaseWorker::requestToken, this, email, password, callback));
}

void WebServerDatabaseWorker::asyncValidateToken(const std::string &email, const Token &token, tokenValidateCallback callback) {
    boost::asio::post(mContext, std::bind(&WebServerDatabaseWorker::validateToken, this, email, token, callback));
}

void WebServerDatabaseWorker::asyncRegisterUser(const std::string &email, const std::string &password, registerUserCallback callback) {
    boost::asio::post(mContext, std::bind(&WebServerDatabaseWorker::registerUser, this, email, password, callback));
}

bool WebServerDatabaseWorker::hasUser(const std::string &email) {
    pqxx::work work(mConnection);
    pqxx::result r = work.exec("select 1 FROM users where email = " + work.quote(email));
    work.commit();

    return (r.size() > 0);
}

bool WebServerDatabaseWorker::checkPassword(const std::string &email, const std::string &password) {
    pqxx::work work(mConnection);
    pqxx::result r = work.exec("select password_hash FROM users where email = " + work.quote(email));
    work.commit();

    if (r.empty())
        return false;

    auto hash = r.front()[0].as<std::string>();
    log_debug().field("hash",hash).msg("Got password hash");
    return Botan::check_bcrypt(password, hash);
}

void WebServerDatabaseWorker::validateToken(const std::string &email, const Token &token, tokenValidateCallback callback) {
    std::string maxTokenExpiration = "2019-02-16 04:05:06";

    pqxx::work work(mConnection);
    pqxx::result r = work.exec("select token_expiration FROM users where email = "
                               + work.quote(email)
                               + " and token = " + work.quote_raw(token.data(), token.size()));
    work.commit();


    if (r.size() == 0) {
        callback(false);
    }
    else {
        log_debug().field("tokenExpiration", r[0][0]).msg("Validated token");
        callback(true);
    }
}

void WebServerDatabaseWorker::registerUser(const std::string &email, const std::string &password, registerUserCallback callback) {
    if (hasUser(email)) {
        callback(false, Token());
        return;
    }

    auto token = generateToken();
    auto tokenExpiration = generateTokenExpiration();

    auto &rng = Botan::system_rng();
    auto passwordHash = Botan::generate_bcrypt(password, rng);
    log_debug().field("passwordHash", passwordHash).msg("Generated password");

    pqxx::work work(mConnection);
    pqxx::result r = work.exec("insert into users (email, password_hash, token, token_expiration) values ("
                               + work.quote(email)
                               + ", " + work.quote(passwordHash)
                               + ", " + work.quote_raw(token.data(), token.size())
                               + ", " + work.quote(tokenExpiration)
                               + ")");
    work.commit();

    callback(true, token);
}

void WebServerDatabaseWorker::requestToken(const std::string &email, const std::string &password, tokenRequestCallback callback) {
    if (!checkPassword(email, password)) {
        callback(false, Token());
        return;
    }

    auto token = generateToken();
    auto tokenExpiration = generateTokenExpiration();

    pqxx::work work(mConnection);
    pqxx::result r = work.exec("update users set token=" + work.quote_raw(token.data(), token.size())
                               + ", token_expiration=" + work.quote(tokenExpiration)
                               + " where email=" + work.quote(email));
    work.commit();

    callback(true, token);
}


Token WebServerDatabaseWorker::generateToken() {
    Token res;
    Botan::system_rng().randomize(res.data(), res.size());

    return res;
}

std::string WebServerDatabaseWorker::generateTokenExpiration() {
    return "2019-02-17";
}

