#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers
#include <pqxx/pqxx>

#include "web/web_types.hpp"

// TODO: Add error codes
class WebServerDatabaseWorker {
public:
    WebServerDatabaseWorker(boost::asio::io_context &masterContext);

    void run();
    void quit();

    typedef std::function<void(bool, const Token&)> registerUserCallback;
    void asyncRegisterUser(const std::string &email, const std::string &password, registerUserCallback callback);

    typedef std::function<void(bool, const Token&)> tokenRequestCallback;
    void asyncRequestToken(const std::string &email, const std::string &password, tokenRequestCallback callback);

    typedef std::function<void(bool)> tokenValidateCallback;
    void asyncValidateToken(const std::string &email, const Token& token, tokenValidateCallback callback);

    // typedef void (*tokenValidateCallback)(bool success);
    // void setWebName(const std::string &email, const std::string &token, tokenValidateCallback callback);
private:
    void requestToken(const std::string &email, const std::string &password, tokenRequestCallback callback);
    void validateToken(const std::string &email, const Token &token, tokenValidateCallback callback);
    void registerUser(const std::string &email, const std::string &password, registerUserCallback callback);

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
