#include "store_handlers/network_server.hpp"

using boost::asio::ip::tcp;

NetworkServer::NetworkServer(int port)
    : mContext()
    , mEndpoint(tcp::v4(), port)
    , mAcceptor(mContext, mEndpoint)
{
    accept();
}

void NetworkServer::run() {
    log_debug().msg("Running network server");
    while (true) {
        try
        {
            mContext.run();
            break; // run() exited normally
        }
        catch (std::exception& e)
        {
            log_error().field("msg", e.what()).msg("NetworkServer caught exception");
        }
    }
}

void NetworkServer::accept() {
    mAcceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (ec) {
            log_error().field("message", ec.message()).msg("Received error code in async_accept");
        }
        else {
            auto connection = std::make_shared<NetworkConnection>(std::move(socket));

            connection->asyncAccept([this, connection](boost::system::error_code ec) {
                if (ec)
                    log_error().field("message", ec.message()).msg("Received error code in connection.asyncAccept");
                else
                    std::make_shared<NetworkParticipant>(std::move(connection), this)->start();
            });
        }

        accept();
    });

}

void NetworkParticipant::start() {
    mServer->join(shared_from_this());
    auto syncMessage = std::make_unique<NetworkMessage>();
    syncMessage->encodeSync(mServer->getTournament(), mServer->getActionStack());
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
            log_error().field("message", ec.message()).msg("Encountered error when writing message. Kicking client");
            mServer->leave(shared_from_this());
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
            log_error().field("message", ec.message()).msg("Encountered error when reading message. Kicking client");
            mServer->leave(shared_from_this());
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::QUIT) {
            mServer->leave(shared_from_this());
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::HANDSHAKE) {
            log_warning().msg("Received HANDSHAKE message after initial handshake");
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::SYNC_ACK) {
            if (!isSyncing())
                log_warning().msg("Received SYNC_ACK from non-syncing client");
            else
                setIsSyncing(false);
        }
        else if (!isSyncing() && mReadMessage->getType() == NetworkMessage::Type::ACTION) {
            mServer->deliver(std::move(mReadMessage), shared_from_this());
        }

        mReadMessage = std::make_unique<NetworkMessage>();

        readMessage();
    });
}

