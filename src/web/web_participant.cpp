#include "web/web_participant.hpp"
#include "web/web_server.hpp"

WebParticipant::WebParticipant(std::shared_ptr<NetworkConnection> connection, WebServer &server, WebServerDatabaseWorker &databaseWorker)
    : mConnection(std::move(connection))
    , mReadMessage(std::make_unique<NetworkMessage>())
    , mServer(server)
    , mDatabaseWorker(databaseWorker)
    , mState(State::NOT_AUTHENTICATED)
{
    asyncAuth();
}

void WebParticipant::asyncAuth() {
    auto self = shared_from_this();

    mConnection->asyncRead(*mReadMessage, [this, self](boost::system::error_code ec) {
        if (ec) {
            log_debug().field("message", ec.message()).msg("Encountered error when reading message. Kicking client");
            mServer.leave(shared_from_this());
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::QUIT) {
            mServer.leave(shared_from_this());
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::REQUEST_WEB_TOKEN) {

        }
        else if (mReadMessage->getType() == NetworkMessage::Type::VALIDATE_WEB_TOKEN) {

        }
        else {
            log_warning().field("type", (unsigned int) mReadMessage->getType()).msg("Received unexpected message type when authenticating");
        }


        mReadMessage = std::make_unique<NetworkMessage>();
        asyncAuth();
    });
}

void WebParticipant::write() {

}

