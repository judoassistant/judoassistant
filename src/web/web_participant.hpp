#pragma once

#include "network/network_connection.hpp"

class WebServer;

// Object passed between web server and web workers encapsulating a client
class WebParticipant : public std::enable_shared_from_this<WebParticipant> {
public:
    WebParticipant(std::shared_ptr<NetworkConnection> connection, WebServer &server);

private:
    std::shared_ptr<NetworkConnection> mConnection;
    WebServer &mServer;
};
