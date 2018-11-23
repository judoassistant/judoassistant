#include "network/network_client.hpp"
#include "stores/qtournament_store.hpp"

using boost::asio::ip::tcp;

NetworkClient::NetworkClient()
    : mId(ClientId::generate())
    , mContext()
    , mReadMessage(std::make_unique<NetworkMessage>())
{
}

void NetworkClient::postSync(std::unique_ptr<TournamentStore> tournament) {
    throw std::runtime_error("Attempted to postSync from network client");
}

void NetworkClient::postAction(ActionId actionId, std::unique_ptr<Action> action) {
    std::shared_ptr<Action> sharedAction = std::move(action);
    mContext.post([this, actionId, sharedAction]() {
        log_debug().field("actionId", actionId).msg("Posting action to network");
        mUnconfirmedActionList.push_back(std::make_pair(actionId, sharedAction));
        mUnconfirmedActionMap[actionId] = std::prev(mUnconfirmedActionList.end());

        if (!mConnection)
            return;

        auto message = std::make_unique<NetworkMessage>();
        message->encodeAction(mId, actionId, std::move(sharedAction));
        deliver(std::move(message));
    });
}

void NetworkClient::postUndo(ActionId actionId) {
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
        auto endpoints = resolver.resolve(host, std::to_string(port));

        mSocket = tcp::socket(mContext);

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

                    QTournamentStore *tournament = new QTournamentStore;
                    ActionList actions;

                    if (!mReadMessage->decodeSync(*tournament, actions)) {
                        emit connectionAttemptFailed();
                        mReadMessage = std::make_unique<NetworkMessage>();
                        return;
                    }

                    // apply the action list but avoid unconfirmed undos
                    std::unordered_set<ActionId> actionIds;
                    auto *unconfirmedUndos = new std::unordered_set<ActionId>;
                    auto *uniqueActions = new UniqueActionList;
                    for (auto &p : actions) {
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
                    auto *unconfirmedActionList = new UniqueActionList;
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
                        message->encodeAction(mId, p.first, p.second);
                        deliver(std::move(message));
                    }

                    for (ActionId actionId : mUnconfirmedUndos) {
                        auto message = std::make_unique<NetworkMessage>();
                        message->encodeUndo(actionId);
                        deliver(std::move(message));
                    }

                    mReadMessage = std::make_unique<NetworkMessage>();
                    emit connectionAttemptSuccess(tournament, uniqueActions, unconfirmedActionList, unconfirmedUndos);

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
            log_error().field("msg", e.what()).msg("NetworkServer caught exception");
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
            QTournamentStore *tournament = new QTournamentStore;
            ActionList *actions = new ActionList;

            if (!mReadMessage->decodeSync(*tournament, *actions)) {
                log_error().msg("Failed to decode sync message. Disconnecting");
                killConnection();
                emit connectionLost();
                return;
            }

            UniqueActionList *uniqueActions = new UniqueActionList;
            for (const auto &p : *actions) {
                auto actionId = p.first;
                auto action = p.second->freshClone();
                action->redo(*tournament);
                uniqueActions->push_back({actionId, std::move(action)});
            }

            auto message = std::make_unique<NetworkMessage>();
            message->encodeSyncAck();
            deliver(std::move(message));

            emit syncReceived(tournament, uniqueActions);
            mUnconfirmedUndos.clear();
            mUnconfirmedActionMap.clear();
            mUnconfirmedActionList.clear();
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::ACTION) {
            ClientId clientId;
            ActionId actionId;
            std::shared_ptr<Action> action;

            if (!mReadMessage->decodeAction(clientId, actionId, action)) {
                log_error().msg("Failed to decode action message. Disconnecting");
                killConnection();
                emit connectionLost();
                return;
            }

            emit actionReceived(actionId, std::move(action));
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::ACTION_ACK) {
            ActionId actionId;
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
            ActionId actionId;
            if (!mReadMessage->decodeUndo(actionId)) {
                log_error().msg("Failed to decode undo. Disconnecting");
                killConnection();
                emit connectionLost();
                return;
            }

            emit undoReceived(actionId);
        }
        else if (mReadMessage->getType() == NetworkMessage::Type::UNDO_ACK) {
            ActionId actionId;
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

