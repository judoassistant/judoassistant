#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers
#include <pqxx/pqxx>

#include "web/web_types.hpp"

class TournamentId;

class WebServerDatabaseWorker {
public:
    WebServerDatabaseWorker(boost::asio::io_context &masterContext);

    void run();
    void quit();

    typedef std::function<void(UserRegistrationResponse, const Token&)> UserRegistrationCallback;
    void asyncRegisterUser(const std::string &email, const std::string &password, UserRegistrationCallback callback);

    typedef std::function<void(TokenRequestResponse, const Token&)> TokenRequestCallback;
    void asyncRequestToken(const std::string &email, const std::string &password, TokenRequestCallback callback);

    typedef std::function<void(TokenValidationResponse)> TokenValidationCallback;
    void asyncValidateToken(const std::string &email, const Token& token, TokenValidationCallback callback);

    typedef std::function<void(WebNameValidationResponse)> WebNameValidationCallback;
    void asyncValidateWebName(const TournamentId &id, const std::string &webName, WebNameValidationCallback callback);

    typedef std::function<void(WebNameRegistrationResponse)> WebNameRegistrationCallback;
    void asyncRegisterWebName(const TournamentId &id, const std::string &webName, WebNameRegistrationCallback callback);

private:
    void registerUser(const std::string &email, const std::string &password, UserRegistrationCallback callback);

    void requestToken(const std::string &email, const std::string &password, TokenRequestCallback callback);
    void validateToken(const std::string &email, const Token &token, TokenValidationCallback callback);

    void validateWebName(const TournamentId &id, const std::string &webName, WebNameValidationCallback callback);
    void registerWebName(const TournamentId &id, const std::string &webName, WebNameRegistrationCallback callback);

    bool hasUser(const std::string &email);
    bool checkPassword(const std::string &email, const std::string &password);
    Token generateToken();
    std::string generateTokenExpiration();

private:
    boost::asio::io_context mContext;
    boost::asio::io_context &mMasterContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;
    pqxx::connection mConnection;
};
