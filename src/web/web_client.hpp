#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers
#include <QString>
#include <QThread>

#include "web/web_name_status.hpp"
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

    void setWebName(TournamentId id, const QString &webName);
    void testWebName(TournamentId id, const QString &webName);

signals:
    void tokenValidationSucceeded();
    void tokenValidationFailed();
    void loginSucceeded(const QString &token);
    void loginFailed(); // TODO: Include reason for failure
    void registrationSucceeded(const QString &token);
    void registrationFailed();
    void disconnected();
    void webNameTested(const QString &webName, WebNameStatus status);
    void webNameChanged(const QString &webNameSuffix);
    void statusChanged(Status status);

private:
    void killConnection();

    boost::asio::io_context mContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;
    std::optional<boost::asio::ip::tcp::socket> mSocket;
    std::optional<NetworkConnection> mConnection;
    bool mQuitPosted;
    Status mStatus;
};

Q_DECLARE_METATYPE(WebClient::Status)
Q_DECLARE_METATYPE(WebNameStatus)

