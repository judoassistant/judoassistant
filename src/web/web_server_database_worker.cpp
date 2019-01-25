#include <botan-2/botan/bcrypt.h>
#include <botan-2/botan/rng.h>
#include <botan-2/botan/system_rng.h>

#include "core/id.hpp"
#include "core/log.hpp"
#include "web/web_server_database_worker.hpp"

// TODO: Move into a json file and read on runtime
constexpr char POSTGRES_CONFIG[] = "user=svendcs host=127.0.0.1 dbname=judoassistant";

WebServerDatabaseWorker::WebServerDatabaseWorker(boost::asio::io_context &masterContext)
    : mContext()
    , mMasterContext(masterContext)
    , mWorkGuard(boost::asio::make_work_guard(mContext))
    , mConnection(POSTGRES_CONFIG)
{

}

void WebServerDatabaseWorker::run() {
    mContext.run();
}

void WebServerDatabaseWorker::quit() {
    mWorkGuard.reset();
}

void WebServerDatabaseWorker::asyncRequestWebToken(const std::string &email, const std::string &password, WebTokenRequestCallback callback) {
    boost::asio::post(mContext, std::bind(&WebServerDatabaseWorker::requestWebToken, this, email, password, callback));
}

void WebServerDatabaseWorker::asyncValidateWebToken(const std::string &email, const WebToken &token, WebTokenValidationCallback callback) {
    boost::asio::post(mContext, std::bind(&WebServerDatabaseWorker::validateWebToken, this, email, token, callback));
}

void WebServerDatabaseWorker::asyncRegisterUser(const std::string &email, const std::string &password, UserRegistrationCallback callback) {
    boost::asio::post(mContext, std::bind(&WebServerDatabaseWorker::registerUser, this, email, password, callback));
}

void WebServerDatabaseWorker::asyncCheckWebName(int userId, const TournamentId &id, const std::string &webName, WebNameCheckCallback callback) {
    boost::asio::post(mContext, std::bind(&WebServerDatabaseWorker::checkWebName, this, userId, id, webName, callback));
}

void WebServerDatabaseWorker::asyncRegisterWebName(int userId, const TournamentId &id, const std::string &webName, WebNameRegistrationCallback callback) {
    boost::asio::post(mContext, std::bind(&WebServerDatabaseWorker::registerWebName, this, userId, id, webName, callback));
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

void WebServerDatabaseWorker::validateWebToken(const std::string &email, const WebToken &token, WebTokenValidationCallback callback) {
    std::string maxWebTokenExpiration = "2019-02-16 04:05:06";

    try {
        pqxx::work work(mConnection);
        pqxx::result r = work.exec("select id FROM users where email = "
                                + work.quote(email)
                                + " and token = " + work.quote_raw(token.data(), token.size()));
        work.commit();

        // TODO: Check web token expiration
        if (r.size() == 0) {
            boost::asio::post(mMasterContext, std::bind(callback, WebTokenValidationResponse::INVALID_TOKEN, std::nullopt));
        }
        else {
            int userId = r[0][0].as<int>();
            boost::asio::post(mMasterContext, std::bind(callback, WebTokenValidationResponse::SUCCESSFUL, userId));
        }
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::post(mMasterContext, std::bind(callback, WebTokenValidationResponse::SERVER_ERROR, std::nullopt));
    }
}

void WebServerDatabaseWorker::registerUser(const std::string &email, const std::string &password, UserRegistrationCallback callback) {
    try {
        if (hasUser(email)) {
            callback(UserRegistrationResponse::EMAIL_EXISTS, std::nullopt, std::nullopt);
            return;
        }

        auto token = generateWebToken();
        auto tokenExpiration = generateWebTokenExpiration();

        auto &rng = Botan::system_rng();
        auto passwordHash = Botan::generate_bcrypt(password, rng);
        log_debug().field("passwordHash", passwordHash).msg("Generated password");

        pqxx::work work(mConnection);
        pqxx::result r = work.exec("insert into users (email, password_hash, token, token_expiration) values ("
                                + work.quote(email)
                                + ", " + work.quote(passwordHash)
                                + ", " + work.quote_raw(token.data(), token.size())
                                + ", " + work.quote(tokenExpiration)
                                + ") returning id");
        work.commit();

        auto userId = r[0][0].as<int>();
        boost::asio::post(mMasterContext, std::bind(callback, UserRegistrationResponse::EMAIL_EXISTS, token, userId));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::post(mMasterContext, std::bind(callback, UserRegistrationResponse::SERVER_ERROR, std::nullopt, std::nullopt));
    }
}

void WebServerDatabaseWorker::requestWebToken(const std::string &email, const std::string &password, WebTokenRequestCallback callback) {
    try {
        if (!checkPassword(email, password)) {
            boost::asio::post(mMasterContext, std::bind(callback, WebTokenRequestResponse::INCORRECT_CREDENTIALS, std::nullopt, std::nullopt));
            return;
        }

        auto token = generateWebToken();
        auto tokenExpiration = generateWebTokenExpiration();

        pqxx::work work(mConnection);
        pqxx::result r = work.exec("update users set token=" + work.quote_raw(token.data(), token.size())
                                + ", token_expiration=" + work.quote(tokenExpiration)
                                + " where email=" + work.quote(email)
                                + " returning id");
        work.commit();

        auto userId = r[0][0].as<int>();
        boost::asio::post(mMasterContext, std::bind(callback, WebTokenRequestResponse::SUCCESSFUL, token, userId));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::post(mMasterContext, std::bind(callback, WebTokenRequestResponse::SERVER_ERROR, std::nullopt, std::nullopt));
    }
}


WebToken WebServerDatabaseWorker::generateWebToken() {
    WebToken res;
    Botan::system_rng().randomize(res.data(), res.size());

    return res;
}

std::string WebServerDatabaseWorker::generateWebTokenExpiration() {
    return "2019-02-17"; // TODO: Implement
}

void WebServerDatabaseWorker::checkWebName(int userId, const TournamentId &id, const std::string &webName, WebNameCheckCallback callback) {
    try {
        pqxx::work work(mConnection);
        pqxx::result r = work.exec("select owner, tournament_id FROM tournaments where web_name = "
                                + work.quote(webName));
        work.commit();

        if (r.empty())
            boost::asio::post(mMasterContext, std::bind(callback, WebNameCheckResponse::FREE));
        else if (r.front()[0].as<int>() != userId)
            boost::asio::post(mMasterContext, std::bind(callback, WebNameCheckResponse::OCCUPIED_OTHER_USER));
        else if (r.front()[1].as<TournamentId::InternalType>() != id.getValue())
            boost::asio::post(mMasterContext, std::bind(callback, WebNameCheckResponse::OCCUPIED_OTHER_TOURNAMENT));
        else
            boost::asio::post(mMasterContext, std::bind(callback, WebNameCheckResponse::OCCUPIED_SAME_TOURNAMENT));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::post(mMasterContext, std::bind(callback, WebNameCheckResponse::SERVER_ERROR));
    }
}

void WebServerDatabaseWorker::registerWebName(int userId, const TournamentId &id, const std::string &webName, WebNameRegistrationCallback callback) {
    try {
        pqxx::work work(mConnection);
        pqxx::result r = work.exec("select owner, tournament_id FROM tournaments where web_name = "
                                + work.quote(webName));
        work.commit();


        if (!r.empty() && r.front()[0].as<int>() != userId) {
            boost::asio::post(mMasterContext, std::bind(callback, WebNameRegistrationResponse::OCCUPIED_OTHER_USER));
            return;
        }

        if (!r.empty() && r.front()[1].as<TournamentId::InternalType>() == id.getValue()) {
            boost::asio::post(mMasterContext, std::bind(callback, WebNameRegistrationResponse::SUCCESSFUL));
            return;
        }

        if (r.empty()) {
            pqxx::work work(mConnection);
            pqxx::result r = work.exec("insert into tournaments (owner, tournament_id, web_name) values (" + work.quote(userId)
                                        + ", " + work.quote(id.getValue())
                                        + ", " + work.quote(webName)
                                        + ")");
            work.commit();
        }
        else {
            pqxx::work work(mConnection);
            pqxx::result r = work.exec("update tournaments set tournament_id=" + work.quote(id.getValue())
                                        + " WHERE web_name=" + work.quote(webName));
            work.commit();
        }

        boost::asio::post(mMasterContext, std::bind(callback, WebNameRegistrationResponse::SUCCESSFUL));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::post(mMasterContext, std::bind(callback, WebNameRegistrationResponse::SERVER_ERROR));
    }
}

