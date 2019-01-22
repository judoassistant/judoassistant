#include "web/web_participant.hpp"
#include "web/web_server.hpp"

WebParticipant::WebParticipant(std::shared_ptr<NetworkConnection> connection, WebServer &server)
    : mConnection(std::move(connection))
    , mServer(server)
{

}

