#include <boost/asio/dispatch.hpp>
#include <botan-2/botan/bcrypt.h>
#include <botan-2/botan/rng.h>
#include <botan-2/botan/system_rng.h>
#include <iomanip>

#include "core/id.hpp"
#include "core/log.hpp"
#include "web/database.hpp"

Database::Database(boost::asio::io_context &context, const std::string &config)
    : mContext(context)
    , mStrand(context)
    , mConnection(config)
{

}

void Database::asyncRequestWebToken(const std::string &email, const std::string &password, WebTokenRequestCallback callback) {
    boost::asio::dispatch(mStrand, std::bind(&Database::requestWebToken, this, email, password, callback));
}

void Database::asyncValidateWebToken(const std::string &email, const WebToken &token, WebTokenValidationCallback callback) {
    boost::asio::dispatch(mStrand, std::bind(&Database::validateWebToken, this, email, token, callback));
}

void Database::asyncRegisterUser(const std::string &email, const std::string &password, UserRegistrationCallback callback) {
    boost::asio::dispatch(mStrand, std::bind(&Database::registerUser, this, email, password, callback));
}

void Database::asyncCheckWebName(int userId, const TournamentId &id, const std::string &webName, WebNameCheckCallback callback) {
    boost::asio::dispatch(mStrand, std::bind(&Database::checkWebName, this, userId, id, webName, callback));
}

void Database::asyncRegisterWebName(int userId, const TournamentId &id, const std::string &webName, WebNameRegistrationCallback callback) {
    boost::asio::dispatch(mStrand, std::bind(&Database::registerWebName, this, userId, id, webName, callback));
}

void Database::asyncSetSynced(const std::string &webName, SyncedSetCallback callback) {
    boost::asio::dispatch(mStrand, std::bind(&Database::setSynced, this, webName, callback));
}

void Database::asyncSetSaveTime(const std::string &webName, std::chrono::system_clock::time_point time, SaveTimeSetCallback callback) {
    boost::asio::dispatch(mStrand, std::bind(&Database::setSaveTime, this, webName, time, callback));
}

void Database::asyncGetSaveStatus(const std::string &webName, SaveStatusGetCallback callback) {
    boost::asio::dispatch(mStrand, std::bind(&Database::getSaveStatus, this, webName, callback));
}

bool Database::hasUser(const std::string &email) {
    pqxx::work work(mConnection);
    pqxx::result r = work.exec("select 1 FROM users where email = " + work.quote(email));
    work.commit();

    return (r.size() > 0);
}

bool Database::checkPassword(const std::string &email, const std::string &password) {
    pqxx::work work(mConnection);
    pqxx::result r = work.exec("select password_hash FROM users where email = " + work.quote(email));
    work.commit();

    if (r.empty())
        return false;

    auto hash = r.front()[0].as<std::string>();
    return Botan::check_bcrypt(password, hash);
}

void Database::validateWebToken(const std::string &email, const WebToken &token, WebTokenValidationCallback callback) {
    std::string maxWebTokenExpiration = "2019-02-16 04:05:06";

    try {
        pqxx::work work(mConnection);
        pqxx::result r = work.exec("select id FROM users where email = "
                                + work.quote(email)
                                + " and token = " + work.quote_raw(token.data(), token.size()));
        work.commit();

        // TODO: Check web token expiration
        if (r.size() == 0) {
            boost::asio::dispatch(mContext, std::bind(callback, WebTokenValidationResponse::INVALID_TOKEN, std::nullopt));
        }
        else {
            int userId = r[0][0].as<int>();
            boost::asio::dispatch(mContext, std::bind(callback, WebTokenValidationResponse::SUCCESSFUL, userId));
        }
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::dispatch(mContext, std::bind(callback, WebTokenValidationResponse::SERVER_ERROR, std::nullopt));
    }
}

void Database::registerUser(const std::string &email, const std::string &password, UserRegistrationCallback callback) {
    try {
        if (hasUser(email)) {
            callback(UserRegistrationResponse::EMAIL_EXISTS, std::nullopt, std::nullopt);
            return;
        }

        auto token = generateWebToken();
        auto tokenExpiration = generateWebTokenExpiration();

        auto &rng = Botan::system_rng();
        auto passwordHash = Botan::generate_bcrypt(password, rng);

        pqxx::work work(mConnection);
        pqxx::result r = work.exec("insert into users (email, password_hash, token, token_expiration) values ("
                                + work.quote(email)
                                + ", " + work.quote(passwordHash)
                                + ", " + work.quote_raw(token.data(), token.size())
                                + ", " + work.quote(tokenExpiration)
                                + ") returning id");
        work.commit();

        auto userId = r[0][0].as<int>();
        boost::asio::dispatch(mContext, std::bind(callback, UserRegistrationResponse::EMAIL_EXISTS, token, userId));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::dispatch(mContext, std::bind(callback, UserRegistrationResponse::SERVER_ERROR, std::nullopt, std::nullopt));
    }
}

void Database::requestWebToken(const std::string &email, const std::string &password, WebTokenRequestCallback callback) {
    try {
        if (!checkPassword(email, password)) {
            boost::asio::dispatch(mContext, std::bind(callback, WebTokenRequestResponse::INCORRECT_CREDENTIALS, std::nullopt, std::nullopt));
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
        boost::asio::dispatch(mContext, std::bind(callback, WebTokenRequestResponse::SUCCESSFUL, token, userId));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::dispatch(mContext, std::bind(callback, WebTokenRequestResponse::SERVER_ERROR, std::nullopt, std::nullopt));
    }
}


WebToken Database::generateWebToken() {
    WebToken res;
    Botan::system_rng().randomize(res.data(), res.size());

    return res;
}

std::string Database::generateWebTokenExpiration() {
    return "2019-02-17";
}

void Database::checkWebName(int userId, const TournamentId &id, const std::string &webName, WebNameCheckCallback callback) {
    try {
        pqxx::work work(mConnection);
        pqxx::result r = work.exec("select owner, tournament_id FROM tournaments where web_name = "
                                + work.quote(webName));
        work.commit();

        if (r.empty())
            boost::asio::dispatch(mContext, std::bind(callback, WebNameCheckResponse::FREE));
        else if (r.front()[0].as<int>() != userId)
            boost::asio::dispatch(mContext, std::bind(callback, WebNameCheckResponse::OCCUPIED_OTHER_USER));
        else if (r.front()[1].as<TournamentId::InternalType>() != id.getValue())
            boost::asio::dispatch(mContext, std::bind(callback, WebNameCheckResponse::OCCUPIED_OTHER_TOURNAMENT));
        else
            boost::asio::dispatch(mContext, std::bind(callback, WebNameCheckResponse::OCCUPIED_SAME_TOURNAMENT));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::dispatch(mContext, std::bind(callback, WebNameCheckResponse::SERVER_ERROR));
    }
}

void Database::registerWebName(int userId, const TournamentId &id, const std::string &webName, WebNameRegistrationCallback callback) {
    try {
        pqxx::work work(mConnection);
        pqxx::result r = work.exec("select owner, tournament_id FROM tournaments where web_name = "
                                + work.quote(webName));
        work.commit();


        if (!r.empty() && r.front()[0].as<int>() != userId) {
            boost::asio::dispatch(mContext, std::bind(callback, WebNameRegistrationResponse::OCCUPIED_OTHER_USER));
            return;
        }

        if (!r.empty() && r.front()[1].as<TournamentId::InternalType>() == id.getValue()) {
            boost::asio::dispatch(mContext, std::bind(callback, WebNameRegistrationResponse::SUCCESSFUL));
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
            pqxx::result r = work.exec("update tournaments set synced=false, tournament_id=" + work.quote(id.getValue())
                                        + " WHERE web_name=" + work.quote(webName));
            work.commit();
        }

        boost::asio::dispatch(mContext, std::bind(callback, WebNameRegistrationResponse::SUCCESSFUL));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::dispatch(mContext, std::bind(callback, WebNameRegistrationResponse::SERVER_ERROR));
    }
}

void Database::setSynced(const std::string &webName, SyncedSetCallback callback) {
    try {
        pqxx::work work(mConnection);
        pqxx::result r = work.exec("update tournaments set synced=true where web_name=" + work.quote(webName));
        work.commit();

        boost::asio::dispatch(mContext, std::bind(callback, (r.affected_rows() > 0)));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::dispatch(mContext, std::bind(callback, false));
    }
}

std::string timeToString(std::chrono::system_clock::time_point time) {
    auto timeT = std::chrono::system_clock::to_time_t(time);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::chrono::system_clock::time_point stringToTime(const std::string &str) {
    std::tm tm = {};
    std::stringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

void Database::setSaveTime(const std::string &webName, std::chrono::system_clock::time_point time, SaveTimeSetCallback callback) {
    try {
        pqxx::work work(mConnection);
        pqxx::result r = work.exec("update tournaments set save_time=" + work.quote(timeToString(time))
                                    + " where web_name=" + work.quote(webName));
        work.commit();

        boost::asio::dispatch(mContext, std::bind(callback, (r.affected_rows() > 0)));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::dispatch(mContext, std::bind(callback, false));
    }
}

void Database::getSaveStatus(const std::string &webName, SaveStatusGetCallback callback) {
    try {
        pqxx::work work(mConnection);
        pqxx::result r = work.exec("select 1 FROM tournaments where web_name = " + work.quote(webName)
                                   + " and synced = true and save_time IS NOT NULL");
        work.commit();

        boost::asio::dispatch(mContext, std::bind(callback, !r.empty()));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::dispatch(mContext, std::bind(callback, false));
    }
}

void Database::asyncUpdateTournament(const std::string &webName, const std::string &name, const std::string &location, const std::string &date, UpdateTournamentCallback callback) {
    boost::asio::dispatch(mStrand, std::bind(&Database::updateTournament, this, webName, name, location, date, callback));
}

void Database::updateTournament(const std::string &webName, const std::string &name, const std::string &location, const std::string &date, UpdateTournamentCallback callback) {
    try {
        pqxx::work work(mConnection);
        pqxx::result r = work.exec("update tournaments set name=" + work.quote(name)
                                    + ", location=" + work.quote(location)
                                    + ", date=" + work.quote(date)
                                    + " where web_name=" + work.quote(webName));
        work.commit();

        boost::asio::dispatch(mContext, std::bind(callback, (r.affected_rows() > 0)));
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("PQXX exception caught");
        boost::asio::dispatch(mContext, std::bind(callback, false));
    }
}
