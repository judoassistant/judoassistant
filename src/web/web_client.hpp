#pragma once

#include <QString>

#include "web/web_client_worker.hpp"
#include "web/url_status.hpp"
#include "id.hpp"

class WebClient {
    enum class Status {
        NOT_CONNECTED,
        CONNECTED,
        CONFIGURED,
    };

    void validateToken(const QString &token);
    void loginUser(const QString &email, const QString &password);
    void registerUser(const QString &email, const QString &password);
    void quit();

    void setUrl(TournamentId id, const QString &url);
    void testUrl(TournamentId id, const QString &url);

    Status getStatus() const;

signals:
    void tokenValidated();
    void tokenValidationFailed();
    void loggedIn(const QString &token);
    void logInFailed();
    void registered(const QString &token);
    void registrationFailed();
    void exited();
    void urlTested(const QString &url, UrlStatus status);

private:
    Status mStatus;
};

