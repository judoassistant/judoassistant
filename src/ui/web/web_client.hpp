#pragma once

#include <QString>
#include <QThread>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <queue>

#include "core/id.hpp"
#include "core/web/web_types.hpp"
#include "core/network/network_connection.hpp"

enum class WebClientState {
    NOT_CONNECTED,
    CONNECTING,
    CONNECTED,
    CONFIGURING,
    CONFIGURED,
    DISCONNECTING
};

class NetworkServer;

class WebClient : public QObject {
Q_OBJECT
public:
    WebClient(boost::asio::io_context &context);
    void setNetworkServer(std::shared_ptr<NetworkServer> networkServer);

    void stop();

    void deliver(std::shared_ptr<NetworkMessage> message);

    void validateToken(const QString &token);
    void loginUser(const QString &email, const QString &password);
    void registerUser(const QString &email, const QString &password);
    void disconnect();

    void registerWebName(TournamentId id, const QString &webName);
    void checkWebName(TournamentId id, const QString &webName);
private:
    typedef std::function<void(boost::system::error_code)> connectionHandler;
    void createConnection(connectionHandler handler);

    void writeMessage();
    void enterConfigured();

signals:
    // TODO: Setup signals when losing connection
    void tokenValidationSucceeded();
    void tokenValidationFailed(WebTokenValidationResponse response);
    void loginSucceeded(const WebToken &token);
    void loginFailed(WebTokenRequestResponse response);
    void registrationSucceeded(const QString &webName);
    void registrationFailed(WebNameRegistrationResponse response);
    void disconnected();
    void webNameChecked(const QString &webName, WebNameCheckResponse status);
    void stateChanged(WebClientState status);

private:
    void killConnection();

    boost::asio::io_context &mContext;
    WebClientState mState;
    std::optional<boost::asio::ip::tcp::socket> mSocket;
    std::optional<NetworkConnection> mConnection;
    bool mDisconnecting;
    std::queue<std::shared_ptr<NetworkMessage>> mWriteQueue;
    std::shared_ptr<NetworkServer> mNetworkServer;
};

Q_DECLARE_METATYPE(WebToken)
Q_DECLARE_METATYPE(UserRegistrationResponse)
Q_DECLARE_METATYPE(WebTokenRequestResponse)
Q_DECLARE_METATYPE(WebTokenValidationResponse)
Q_DECLARE_METATYPE(WebNameCheckResponse)
Q_DECLARE_METATYPE(WebNameRegistrationResponse)

Q_DECLARE_METATYPE(WebClientState)
