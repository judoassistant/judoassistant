#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers
#include <QString>
#include <QThread>

#include "web/url_status.hpp"
#include "network/network_connection.hpp"
#include "id.hpp"

// TODO: Make thread classes more consistent
class WebClient : public QThread {
Q_OBJECT
public:
    enum class Status {
        NOT_CONNECTED,
        CONNECTING,
        CONNECTED,
        CONFIGURING,
        CONFIGURED,
        DISCONNECTING
    };

    WebClient();

    void run() override;
    void quit();

    void validateToken(const QString &token);
    void loginUser(const QString &email, const QString &password);
    void registerUser(const QString &email, const QString &password);
    void disconnect();

    void setUrl(TournamentId id, const QString &url);
    void testUrl(TournamentId id, const QString &url);

signals:
    void tokenValidationSucceeded();
    void tokenValidationFailed();
    void logInSucceeded(const QString &token);
    void logInFailed();
    void registrationSucceeded(const QString &token);
    void registrationFailed();
    void disconnected();
    void urlTested(const QString &url, UrlStatus status);
    void statusChanged(Status status);

private:
    boost::asio::io_context mContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;
    std::optional<boost::asio::ip::tcp::socket> mSocket;
    std::optional<NetworkConnection> mConnection;
    bool mQuitPosted;
    Status mStatus;
};

Q_DECLARE_METATYPE(WebClient::Status)
Q_DECLARE_METATYPE(UrlStatus)

