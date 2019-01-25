#include "core/network/network_connection.hpp"
#include "core/network/network_message.hpp"
#include "ui/network/network_participant.hpp"
#include "ui/network/network_server.hpp"

NetworkParticipant::NetworkParticipant(std::shared_ptr<NetworkConnection> connection, NetworkServer &server)
    : mConnection(std::move(connection))
    , mServer(server)
    , mReadMessage(std::make_unique<NetworkMessage>())
    , mIsSyncing(true)
{}

void NetworkParticipant::start() {
    mServer.join(shared_from_this());
    auto syncMessage = std::make_unique<NetworkMessage>();
    syncMessage->encodeSync(*(mServer.getTournament()), mServer.getActionStack());
    deliver(std::move(syncMessage));

    readMessage();
}

void NetworkParticipant::deliver(std::shared_ptr<NetworkMessage> message) {
    bool writeInProgress = !mMessageQueue.empty();
    mMessageQueue.push(std::move(message));

    if (!writeInProgress)
        writeMessage();
}

void NetworkParticipant::writeMessage() {
    auto self = shared_from_this();

    mConnection->asyncWrite(*(mMessageQueue.front()), [this, self](boost::system::error_code ec) {
        if (ec) {
            log_warning().field("message", ec.message()).msg("Encountered error when writing message. Kicking client");
            mServer.leave(shared_from_this());
            return;
        }

        mMessageQueue.pop();
        if (!mMessageQueue.empty())
            writeMessage();
    });
}

void NetworkParticipant::readMessage() {
    auto self = shared_from_this();

    mConnection->asyncRead(*mReadMessage, [this, self](boost::system::error_code ec) {
        if (ec) {
            log_warning().field("message", ec.message()).msg("Encountered error when reading message. Kicking client");
            mServer.leave(shared_from_this());
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::QUIT) {
            mServer.leave(shared_from_this());
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::HANDSHAKE) {
            log_warning().msg("Received HANDSHAKE message after initial handshake");
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::SYNC) {
            log_warning().msg("Received SYNC from client");
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::ACTION_ACK) {
            log_warning().msg("Received ACTION_ACK from client");
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::UNDO_ACK) {
            log_warning().msg("Received UNDO_ACK from client");
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::SYNC_ACK) {
            if (!isSyncing())
                log_warning().msg("Received SYNC_ACK from non-syncing client");
            else
                setIsSyncing(false);
        }
        else if (!isSyncing() && mReadMessage->getType() == NetworkMessage::Type::ACTION) {
            mServer.deliverAction(std::move(mReadMessage), shared_from_this());
        }
        else if (!isSyncing() && mReadMessage->getType() == NetworkMessage::Type::UNDO) {
            mServer.deliverUndo(std::move(mReadMessage), shared_from_this());
        }

        mReadMessage = std::make_unique<NetworkMessage>();

        readMessage();
    });
}

void NetworkParticipant::setIsSyncing(bool value) {
    mIsSyncing = value;
}

bool NetworkParticipant::isSyncing() const {
    return mIsSyncing;
}

