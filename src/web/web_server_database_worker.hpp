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

    // TODO: Use userid in interface
    typedef std::function<void(UserRegistrationResponse, const std::optional<WebToken>&, std::optional<int>)> UserRegistrationCallback;
    void asyncRegisterUser(const std::string &email, const std::string &password, UserRegistrationCallback callback);

    typedef std::function<void(WebTokenRequestResponse, const std::optional<WebToken>&, std::optional<int>)> WebTokenRequestCallback;
    void asyncRequestWebToken(const std::string &email, const std::string &password, WebTokenRequestCallback callback);

    typedef std::function<void(WebTokenValidationResponse, std::optional<int>)> WebTokenValidationCallback;
    void asyncValidateWebToken(const std::string &email, const WebToken& token, WebTokenValidationCallback callback);

    typedef std::function<void(WebNameCheckResponse)> WebNameCheckCallback;
    void asyncCheckWebName(int userId, const TournamentId &id, const std::string &webName, WebNameCheckCallback callback);

    typedef std::function<void(WebNameRegistrationResponse)> WebNameRegistrationCallback;
    void asyncRegisterWebName(int userId, const TournamentId &id, const std::string &webName, WebNameRegistrationCallback callback);

private:
    void registerUser(const std::string &email, const std::string &password, UserRegistrationCallback callback);

    void requestWebToken(const std::string &email, const std::string &password, WebTokenRequestCallback callback);
    void validateWebToken(const std::string &email, const WebToken &token, WebTokenValidationCallback callback);

    void checkWebName(int userId, const TournamentId &id, const std::string &webName, WebNameCheckCallback callback);
    void registerWebName(int userId, const TournamentId &id, const std::string &webName, WebNameRegistrationCallback callback);

    bool hasUser(const std::string &email);
    bool checkPassword(const std::string &email, const std::string &password);
    WebToken generateWebToken();
    std::string generateWebTokenExpiration();

private:
    boost::asio::io_context mContext;
    boost::asio::io_context &mMasterContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;
    pqxx::connection mConnection;
};
