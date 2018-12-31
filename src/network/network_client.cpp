#include "network/network_client.hpp"
#include "stores/qtournament_store.hpp"

using boost::asio::ip::tcp;

NetworkClient::NetworkClient()
    : mContext()
    , mWorkGuard(boost::asio::make_work_guard(mContext))
    , mReadMessage(std::make_unique<NetworkMessage>())
{
}

void NetworkClient::postSync(std::unique_ptr<TournamentStore> tournament) {
    throw std::runtime_error("Attempted to postSync from network client");
}

void NetworkClient::postAction(ClientActionId actionId, std::unique_ptr<Action> action) {
    std::shared_ptr<Action> sharedAction = std::move(action);
    mContext.post([this, actionId, sharedAction]() {
        log_debug().field("actionId", actionId).msg("Posting action to network");
        mUnconfirmedActionList.push_back(std::make_pair(actionId, sharedAction));
        mUnconfirmedActionMap[actionId] = std::prev(mUnconfirmedActionList.end());

        if (!mConnection)
            return;

        auto message = std::make_unique<NetworkMessage>();
        message->encodeAction(actionId, std::move(sharedAction));
        deliver(std::move(message));
    });
}

void NetworkClient::postUndo(ClientActionId actionId) {
    mContext.post([this, actionId]() {
        log_debug().field("actionId", actionId).msg("Posting undo to network");
        mUnconfirmedUndos.insert(actionId);

        if (!mConnection)
            return;

        auto message = std::make_unique<NetworkMessage>();
        message->encodeUndo(actionId);
        deliver(std::move(message));
    });
}

void NetworkClient::deliver(std::unique_ptr<NetworkMessage> message) {
    bool writeInProgress = !mWriteQueue.empty();
    mWriteQueue.push(std::move(message));
    if (!writeInProgress)
        writeMessage();
}

void NetworkClient::postConnect(const std::string &host, unsigned int port) {
    mContext.post([this, host, port]() {
        log_debug().msg("Connecting..");
        mQuitPosted = false;
        tcp::resolver resolver(mContext);
        tcp::resolver::results_type endpoints;
        try {
            endpoints = resolver.resolve(host, std::to_string(port));
        }
        catch(const std::exception &e) {
            log_error().field("message", e.what()).msg("Encountered resolving host. Failing");
            emit connectionAttemptFailed();
            return;
        }

        mSocket = tcp::socket(mContext);

        // TODO: Somehow kill when taking too long
        boost::asio::async_connect(*mSocket, endpoints,
        [this](boost::system::error_code ec, tcp::endpoint)
        {
          if (ec) {
            log_error().field("message", ec.message()).msg("Encountered error when connecting. Killing connection");
            killConnection();
            emit connectionAttemptFailed();
            return;
          }
          else {
            mConnection = NetworkConnection(std::move(*mSocket));
            mSocket.reset();
            mConnection->asyncJoin([this](boost::system::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Encountered error when connecting. Killing connection");
                    killConnection();
                    emit connectionAttemptFailed();
                    return;
                }

                mConnection->asyncRead(*mReadMessage, [this](boost::system::error_code ec) {
                    if (mReadMessage->getType() != NetworkMessage::Type::SYNC) {
                        log_error().msg("Did not immediately receive sync on connection. Killing connection");
                        killConnection();
                        emit connectionAttemptFailed();
                        return;
                    }

                    auto tournament = std::make_unique<QTournamentStore>();
                    SharedActionList sharedActions;

                    if (!mReadMessage->decodeSync(*tournament, sharedActions)) {
                        log_debug().msg("Failed decoding sync");
                        killConnection();
                        emit connectionAttemptFailed();
                        return;
                    }

                    // apply the action list but avoid unconfirmed undos
                    std::unordered_set<ClientActionId> actionIds;
                    auto unconfirmedUndos = std::make_unique<std::unordered_set<ClientActionId>>();
                    auto uniqueActions = std::make_unique<UniqueActionList>();
                    for (auto &p : sharedActions) {
                        auto actionId = p.first;
                        actionIds.insert(actionId);

                        auto action = p.second->freshClone();

                        if (mUnconfirmedUndos.find(actionId) != mUnconfirmedUndos.end())
                            unconfirmedUndos->insert(actionId);
                        else
                            action->redo(*tournament);

                        uniqueActions->push_back({actionId, std::move(action)});
                    }

                    mUnconfirmedUndos = *unconfirmedUndos;

                    // Remove already applied actions from unconfirmed list and map
                    for (auto it = mUnconfirmedActionList.begin(); it != mUnconfirmedActionList.end(); ) {
                        auto next = std::next(it);
                        auto actionId = it->first;

                        if (actionIds.find(actionId) == actionIds.end()) {
                            mUnconfirmedActionMap.erase(actionId);
                            mUnconfirmedActionList.erase(it);
                        }

                        it = next;
                    }

                    // Copy unconfirmed set and unconfirmed action list
                    auto unconfirmedActionList = std::make_unique<UniqueActionList>();
                    for (const auto &p : mUnconfirmedActionList) {
                        auto actionId = p.first;
                        auto action = p.second->freshClone();
                        action->redo(*tournament);
                        unconfirmedActionList->push_back({actionId, std::move(action)});
                    }

                    // Send sync acknowledgement and unconfirmed actions
                    {
                        auto message = std::make_unique<NetworkMessage>();
                        message->encodeSyncAck();
                        deliver(std::move(message));
                    }

                    for (const auto &p : mUnconfirmedActionList) {
                        auto message = std::make_unique<NetworkMessage>();
                        message->encodeAction(p.first, p.second);
                        deliver(std::move(message));
                    }

                    for (ClientActionId actionId : mUnconfirmedUndos) {
                        auto message = std::make_unique<NetworkMessage>();
                        message->encodeUndo(actionId);
                        deliver(std::move(message));
                    }

                    emit syncReceived(std::make_shared<SyncPayload>(std::move(tournament), std::move(uniqueActions), std::move(unconfirmedActionList), std::move(unconfirmedUndos)));
                    emit connectionAttemptSucceeded();

                    mReadMessage = std::make_unique<NetworkMessage>();
                    readMessage();
                });
            });
          }
        });
    });
}

void NetworkClient::start() {
    QThread::start();
}

void NetworkClient::quit() {
    postDisconnect();
    mWorkGuard.reset();
    QThread::quit();
}

void NetworkClient::postDisconnect() {
    mContext.post([this]() {
        mQuitPosted = true; // writeMessage will kill the connection when finished
        auto message = std::make_unique<NetworkMessage>();
        message->encodeQuit();
        deliver(std::move(message));
    });
}

void NetworkClient::run() {
    while (true) {
        try
        {
            mContext.run();
            break; // run() exited normally
        }
        catch (std::exception& e)
        {
            log_error().field("msg", e.what()).msg("NetworkClient caught exception");
        }
    }

    log_info().msg("NetworkClient::run finished");
}

void NetworkClient::writeMessage() {
    mConnection->asyncWrite(*(mWriteQueue.front()), [this](boost::system::error_code ec) {
        if (ec) {
            if (!mConnection) // Was killed by reader
                return;
            log_error().field("message", ec.message()).msg("Encountered error when reading message. Disconnecting");
            while(!mWriteQueue.empty())
                mWriteQueue.pop();
            killConnection();
            emit connectionLost();
            return;
        }

        mWriteQueue.pop();
        if (!mWriteQueue.empty()) {
            writeMessage();
        }
        else if(mQuitPosted) {
            killConnection();
            emit connectionShutdown();
            mQuitPosted = false;
        }
    });
}

void NetworkClient::readMessage() {
    mConnection->asyncRead(*mReadMessage, [this](boost::system::error_code ec) {
        if (ec) {
            if (mQuitPosted) // Everything is handled by asyncRead
                return;
            log_error().field("message", ec.message()).msg("Encountered error when reading message. Disconnecting");
            killConnection();
            emit connectionLost();
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::QUIT) {
            log_info().msg("Received quit message. Disconnecting.");
            killConnection();
            emit connectionShutdown();
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::HANDSHAKE) {
            log_warning().msg("Received HANDSHAKE message after initial handshake");
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::SYNC_ACK) {
            log_warning().msg("Received SYNC_ACK from server");
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::SYNC) {
            auto tournament = std::make_unique<QTournamentStore>();
            SharedActionList sharedActions;

            if (!mReadMessage->decodeSync(*tournament, sharedActions)) {
                log_error().msg("Failed to decode sync message. Disconnecting");
                killConnection();
                emit connectionLost();
                return;
            }

            auto uniqueActions = std::make_unique<UniqueActionList>();
            for (const auto &p : sharedActions) {
                auto actionId = p.first;
                auto action = p.second->freshClone();
                action->redo(*tournament);
                uniqueActions->push_back({actionId, std::move(action)});
            }

            auto message = std::make_unique<NetworkMessage>();
            message->encodeSyncAck();
            deliver(std::move(message));

            mUnconfirmedUndos.clear();
            mUnconfirmedActionMap.clear();

            emit syncReceived(std::make_shared<SyncPayload>(std::move(tournament), std::move(uniqueActions), std::make_unique<UniqueActionList>(), std::make_unique<std::unordered_set<ClientActionId>>()));
            mUnconfirmedUndos.clear();
            mUnconfirmedActionMap.clear();
            mUnconfirmedActionList.clear();
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::ACTION) {
            ClientActionId actionId;
            std::shared_ptr<Action> action;

            if (!mReadMessage->decodeAction(actionId, action)) {
                log_error().msg("Failed to decode action message. Disconnecting");
                killConnection();
                emit connectionLost();
                return;
            }

            emit actionReceived(actionId, std::move(action));
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::ACTION_ACK) {
            ClientActionId actionId;
            if (!mReadMessage->decodeActionAck(actionId)) {
                log_error().msg("Failed to decode action ack. Disconnecting");
                killConnection();
                emit connectionLost();
                return;
            }

            auto it = mUnconfirmedActionMap.find(actionId);
            if (it != mUnconfirmedActionMap.end()) { // Might have been an acknowledge of a "recovery message"
                mUnconfirmedActionList.erase(it->second);
                mUnconfirmedActionMap.erase(it);

                emit actionConfirmReceived(actionId);
            }
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::UNDO) {
            ClientActionId actionId;
            if (!mReadMessage->decodeUndo(actionId)) {
                log_error().msg("Failed to decode undo. Disconnecting");
                killConnection();
                emit connectionLost();
                return;
            }

            emit undoReceived(actionId);
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::UNDO_ACK) {
            ClientActionId actionId;
            if (!mReadMessage->decodeUndoAck(actionId)) {
                log_error().msg("Failed to decode undo ack. Disconnecting");
                killConnection();
                emit connectionLost();
                return;
            }

            auto it = mUnconfirmedUndos.find(actionId);

            if (it != mUnconfirmedUndos.end()) { // Might have been an acknowledge of a "recovery message"
                mUnconfirmedUndos.erase(it);
                emit undoConfirmReceived(actionId);
            }
        }

        mReadMessage = std::make_unique<NetworkMessage>();

        if (!mQuitPosted)
            readMessage();
    });
}

void NetworkClient::killConnection() {
    mConnection.reset();
    mSocket.reset();
    while (!mWriteQueue.empty())
        mWriteQueue.pop();
    mReadMessage = std::make_unique<NetworkMessage>();
}

