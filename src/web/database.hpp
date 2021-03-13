#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <chrono>
#include <pqxx/pqxx>

#include "core/web/web_types.hpp"

class TournamentId;

struct TournamentListing {
    std::string webName;
    std::string name;
    std::string location;
    std::string date;
};

class Database {
public:
    Database(boost::asio::io_context &context, const std::string &config);

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

    typedef std::function<void(bool)> SyncedSetCallback;
    void asyncSetSynced(const std::string &webName, SyncedSetCallback callback);

    typedef std::function<void(bool)> SaveTimeSetCallback;
    void asyncSetSaveTime(const std::string &webName, std::chrono::system_clock::time_point time, SaveTimeSetCallback callback);

    typedef std::function<void(bool)> SaveStatusGetCallback;
    void asyncGetSaveStatus(const std::string &webName, SaveStatusGetCallback callback);

    typedef std::function<void(bool)> UpdateTournamentCallback;
    void asyncUpdateTournament(const std::string &webName, const std::string &name, const std::string &location, const std::string &date, UpdateTournamentCallback callback);

    typedef std::function<void(bool, std::vector<TournamentListing>, std::vector<TournamentListing>)> ListTournamentsCallback;
    void asyncListTournaments(ListTournamentsCallback callback);

private:
    void registerUser(const std::string &email, const std::string &password, UserRegistrationCallback callback);

    void requestWebToken(const std::string &email, const std::string &password, WebTokenRequestCallback callback);
    void validateWebToken(const std::string &email, const WebToken &token, WebTokenValidationCallback callback);

    void checkWebName(int userId, const TournamentId &id, const std::string &webName, WebNameCheckCallback callback);
    void registerWebName(int userId, const TournamentId &id, const std::string &webName, WebNameRegistrationCallback callback);

    void setSynced(const std::string &webName, SyncedSetCallback callback);
    void setSaveTime(const std::string &webName, std::chrono::system_clock::time_point time, SaveTimeSetCallback callback);

    void getSaveStatus(const std::string &webName, SaveStatusGetCallback callback);

    void updateTournament(const std::string &webName, const std::string &name, const std::string &location, const std::string &date, UpdateTournamentCallback callback);

    void listTournaments(ListTournamentsCallback callback);

    bool hasUser(const std::string &email);
    bool checkPassword(const std::string &email, const std::string &password);
    WebToken generateWebToken();

    std::string generateWebTokenExpiration();
    bool validateWebTokenExpiration(const std::string &expiration);

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    pqxx::connection mConnection;
};

