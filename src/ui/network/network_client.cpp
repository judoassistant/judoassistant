#include <boost/asio/connect.hpp>

#include "ui/network/network_client.hpp"
#include "ui/stores/qtournament_store.hpp"

using boost::asio::ip::tcp;

NetworkClient::NetworkClient(boost::asio::io_context &context)
    : mState(NetworkClientState::NOT_CONNECTED)
    , mContext(context)
    , mReadMessage(std::make_unique<NetworkMessage>())
{
    qRegisterMetaType<NetworkClientState>();
    qRegisterMetaType<std::chrono::milliseconds>();
}

void NetworkClient::postSync(std::unique_ptr<TournamentStore> tournament) {
    throw std::runtime_error("Attempted to postSync from network client");
}

void NetworkClient::postAction(ClientActionId actionId, std::unique_ptr<Action> action) {
    std::shared_ptr<Action> sharedAction = std::move(action);
    mContext.post([this, actionId, sharedAction]() {
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

void NetworkClient::connect(const std::string &host, unsigned int port) {
    mContext.post([this, host, port]() {
        emit stateChanged(mState = NetworkClientState::CONNECTING);
        mQuitPosted = false;
        tcp::resolver resolver(mContext);
        tcp::resolver::results_type endpoints;
        try {
            endpoints = resolver.resolve(host, std::to_string(port));
        }
        catch(const std::exception &e) {
            log_error().field("message", e.what()).msg("Encountered resolving host. Failing");
            emit connectionAttemptFailed();
            emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
            return;
        }

        mSocket = tcp::socket(mContext);

        // TODO: Somehow kill when taking too long
        boost::asio::async_connect(*mSocket, endpoints, [this](boost::system::error_code ec, tcp::endpoint) {
            if (ec) {
                log_error().field("message", ec.message()).msg("Encountered error when connecting. Killing connection");
                killConnection();
                emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
                emit connectionAttemptFailed();
                return;
            }

            mConnection = NetworkConnection(std::move(*mSocket));
            mSocket.reset();
            connectJoin();
        });
    });
}

void NetworkClient::disconnect() {
    mContext.post([this]() {
        if (mState != NetworkClientState::CONNECTED) {
            log_warning().msg("Tried to disconnect not-connected network client");
            return;
        }

        emit stateChanged(mState = NetworkClientState::DISCONNECTING);
        mQuitPosted = true; // writeMessage will kill the connection when finished
        auto message = std::make_unique<NetworkMessage>();
        message->encodeQuit();
        deliver(std::move(message));
    });
}

void NetworkClient::stop() {
    mContext.post([this]() {
        if (mState == NetworkClientState::NOT_CONNECTED)
            return;

        disconnect();
    });
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
            emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
            return;
        }

        mWriteQueue.pop();
        if (!mWriteQueue.empty()) {
            writeMessage();
        }
        else if(mQuitPosted) {
            killConnection();
            emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
            emit connectionShutdown();
            mQuitPosted = false;
        }
    });
}

void NetworkClient::connectIdle() {
    mConnection->asyncRead(*mReadMessage, [this](boost::system::error_code ec) {
        if (ec) {
            if (mQuitPosted) // Everything is handled by asyncRead
                return;
            log_error().field("message", ec.message()).msg("Encountered error when reading message. Disconnecting");
            killConnection();
            emit connectionLost();
            emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::QUIT) {
            log_info().msg("Received quit message. Disconnecting.");
            killConnection();
            emit connectionShutdown();
            emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
            return;
        }

        if (mReadMessage->getType() == NetworkMessage::Type::SYNC) {
            auto tournament = std::make_unique<QTournamentStore>();
            SharedActionList sharedActions;

            if (!mReadMessage->decodeSync(*tournament, sharedActions)) {
                log_error().msg("Failed to decode sync message. Disconnecting");
                killConnection();
                emit connectionLost();
                emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
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
                emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
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
                emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
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
                emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
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
                emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
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
            connectIdle();
    });
}

void NetworkClient::killConnection() {
    mConnection.reset();
    mSocket.reset();
    while (!mWriteQueue.empty())
        mWriteQueue.pop();
    mReadMessage = std::make_unique<NetworkMessage>();
}

void NetworkClient::connectJoin() {
    mConnection->asyncJoin([this](boost::system::error_code ec) {
        if (ec) {
            log_error().field("message", ec.message()).msg("Encountered error when connecting. Killing connection");
            killConnection();
            emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
            emit connectionAttemptFailed();
            return;
        }

        connectSynchronizeClocks();
    });
}

void NetworkClient::connectSynchronizeClocks() {
    // Approximate the different between local and master clock
    auto syncRequestMessage = std::make_shared<NetworkMessage>();
    syncRequestMessage->encodeClockSyncRequest();
    auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    mConnection->asyncWrite(*syncRequestMessage, [this, syncRequestMessage, t1](boost::system::error_code ec) {
        if (ec) {
            log_error().field("message", ec.message()).msg("Encountered error when sending clock sync request. Killing connection");
            killConnection();
            emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
            emit connectionAttemptFailed();
            return;
        }

        mReadMessage = std::make_unique<NetworkMessage>();
        mConnection->asyncRead(*mReadMessage, [this, t1](boost::system::error_code ec) {
            auto t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

            if (ec || mReadMessage->getType() != NetworkMessage::Type::CLOCK_SYNC) {
                log_error().msg("Encountered error when reading clock sync message. Killing connection");
                killConnection();
                emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
                emit connectionAttemptFailed();
                return;
            }

            std::chrono::milliseconds p1;
            if (!mReadMessage->decodeClockSync(p1)) {
                killConnection();
                emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
                emit connectionAttemptFailed();
                return;
            }

            auto diff = p1 - (t1 + t2)/2;
            emit clockSynchronized(diff);

            connectSync();
        });
    });
}

void NetworkClient::connectSync() {
    mReadMessage = std::make_unique<NetworkMessage>();
    mConnection->asyncRead(*mReadMessage, [this](boost::system::error_code ec) {
        if (mReadMessage->getType() != NetworkMessage::Type::SYNC) {
            log_error().msg("Did not immediately receive sync on connection. Killing connection");
            killConnection();
            emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
            emit connectionAttemptFailed();
            return;
        }

        auto tournament = std::make_unique<QTournamentStore>();
        SharedActionList sharedActions;

        if (!mReadMessage->decodeSync(*tournament, sharedActions)) {
            log_error().msg("Failed decoding sync");
            killConnection();
            emit stateChanged(mState = NetworkClientState::NOT_CONNECTED);
            emit connectionAttemptFailed();
            return;
        }

        // Calculate the new action list and unconfirmed undos.
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

        // Calculate the unconfirmed action list
        SharedActionList sharedUnconfirmedActionList;
        auto uniqueUnconfirmedActionList = std::make_unique<UniqueActionList>();
        std::unordered_map<ClientActionId, SharedActionList::iterator> unconfirmedActionMap;

        for (auto it = mUnconfirmedActionList.begin(); it != mUnconfirmedActionList.end(); ++it) {
            const auto actionId = it->first;

            if (actionIds.find(actionId) != actionIds.end()) // Already applied
                continue;

            if (mUnconfirmedUndos.find(actionId) != mUnconfirmedUndos.end()) // Undone locally but never send. Delete action
                continue;

            const auto &action = it->second;

            { // Update the locally stored actions
                sharedUnconfirmedActionList.emplace_back(actionId, action->freshClone());
                unconfirmedActionMap.emplace(actionId, std::prev(sharedUnconfirmedActionList.end()));
            }

            { // Update the actions to be emitted
                auto actionClone = action->freshClone();
                actionClone->redo(*tournament);
                uniqueUnconfirmedActionList->emplace_back(actionId, std::move(actionClone));
            }
        }

        // Update the field variables
        mUnconfirmedActionList = std::move(sharedUnconfirmedActionList);
        mUnconfirmedActionMap = std::move(unconfirmedActionMap);
        mUnconfirmedUndos = *unconfirmedUndos; // This is copied, since it needs to be emitted

        // Send sync acknowledgement and unconfirmed actions
        {
            auto message = std::make_unique<NetworkMessage>();
            message->encodeSyncAck();
            deliver(std::move(message));
        }

        for (ClientActionId actionId : mUnconfirmedUndos) {
            auto message = std::make_unique<NetworkMessage>();
            message->encodeUndo(actionId);
            deliver(std::move(message));
        }

        for (const auto &p : mUnconfirmedActionList) {
            auto message = std::make_unique<NetworkMessage>();
            message->encodeAction(p.first, p.second);
            deliver(std::move(message));
        }

        // Emit signals
        emit syncReceived(std::make_shared<SyncPayload>(std::move(tournament), std::move(uniqueActions), std::move(uniqueUnconfirmedActionList), std::move(unconfirmedUndos)));
        emit stateChanged(mState = NetworkClientState::CONNECTED);
        emit connectionAttemptSucceeded();

        mReadMessage = std::make_unique<NetworkMessage>();
        connectIdle();
    });
}

