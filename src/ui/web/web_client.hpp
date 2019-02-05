#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers
#include <QString>
#include <QThread>

#include "core/id.hpp"
#include "core/web/web_types.hpp"
#include "core/network/network_connection.hpp"

// TODO: Make thread classes more consistent
class WebClient : public QObject {
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

    stop();

    void validateToken(const QString &token);
    void loginUser(const QString &email, const QString &password);
    void registerUser(const QString &email, const QString &password);
    void disconnect();

    void registerWebName(TournamentId id, const QString &webName);
    void checkWebName(TournamentId id, const QString &webName);

private:
    typedef std::function<void(boost::system::error_code)> connectionHandler;
    void createConnection(connectionHandler handler);
signals:
    void tokenValidationSucceeded();
    void tokenValidationFailed(WebTokenValidationResponse response);
    void loginSucceeded(const WebToken &token);
    void loginFailed(WebTokenRequestResponse response);
    void registrationSucceeded(const QString &webName);
    void registrationFailed(WebNameRegistrationResponse response);
    void disconnected();
    void webNameChecked(const QString &webName, WebNameCheckResponse status);
    void statusChanged(WebClient::Status status);

private:
    void killConnection();

    boost::asio::io_context mContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;
    std::optional<boost::asio::ip::tcp::socket> mSocket;
    std::optional<NetworkConnection> mConnection;
    bool mQuitPosted;
    Status mStatus;
};

Q_DECLARE_METATYPE(WebToken)
Q_DECLARE_METATYPE(UserRegistrationResponse)
Q_DECLARE_METATYPE(WebTokenRequestResponse)
Q_DECLARE_METATYPE(WebTokenValidationResponse)
Q_DECLARE_METATYPE(WebNameCheckResponse)
Q_DECLARE_METATYPE(WebNameRegistrationResponse)

Q_DECLARE_METATYPE(WebClient::Status)
