#include "core/network/network_connection.hpp"
#include "core/network/network_message.hpp"
#include "ui/network/network_participant.hpp"
#include "ui/network/network_server.hpp"
#include "ui/stores/qtournament_store.hpp"

using boost::asio::ip::tcp;

NetworkServer::NetworkServer(int port)
    : mContext()
    , mEndpoint(tcp::v4(), port)
    , mAcceptor(mContext, mEndpoint)
{
    accept();
}

void NetworkServer::run() {
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
                if (ec) {
                    log_error().field("message", ec.message()).msg("Received error code in connection.asyncAccept");
                }
                else {
                    std::make_shared<NetworkParticipant>(std::move(connection), *this)->start();
                }
            });
        }

        if (mAcceptor.is_open())
            accept();
    });
}

void NetworkServer::postSync(std::unique_ptr<TournamentStore> tournament) {
    std::shared_ptr<TournamentStore> ptr = std::move(tournament);
    mContext.post([this, ptr]() {
        mTournament = std::move(ptr);
        mActionStack.clear();
        mActionMap.clear();

        auto message = std::make_shared<NetworkMessage>();
        message->encodeSync(*mTournament, mActionStack);

        for (auto & participant : mParticipants) {
            participant->setIsSyncing(true);
            participant->deliver(message);
        }

        emit syncConfirmed();
    });
}

void NetworkServer::postAction(ClientActionId actionId, std::unique_ptr<Action> action) {
    std::shared_ptr<Action> sharedAction = std::move(action);
    mContext.post([this, actionId, sharedAction]() {
        mActionStack.push_back(std::make_pair(actionId, sharedAction));
        mActionMap[actionId] = std::prev(mActionStack.end());

        if (mActionStack.size() > ACTION_STACK_MAX_SIZE) {
            mActionStack.front().second->redo(*mTournament);
            mActionMap.erase(mActionStack.front().first);
            mActionStack.pop_front();
        }

        auto message = std::make_unique<NetworkMessage>();
        message->encodeAction(actionId, std::move(sharedAction));

        deliver(std::move(message));

        emit actionConfirmReceived(actionId);
    });
}

void NetworkServer::postUndo(ClientActionId actionId) {
    mContext.post([this, actionId]() {
        // Only deliver if action is not already undone
        auto it = mActionMap.find(actionId);
        if (it != mActionMap.end()) {
            mActionStack.erase(it->second);
            mActionMap.erase(it);

            auto message = std::make_unique<NetworkMessage>();
            message->encodeUndo(actionId);

            deliver(std::move(message));
        }

        emit undoConfirmReceived(actionId);
    });
}

void NetworkServer::postQuit() {
    mContext.post([this]() {
        mAcceptor.close();

        auto message = std::make_shared<NetworkMessage>();
        message->encodeQuit();

        auto it = mParticipants.begin();
        while (it != mParticipants.end()) {
            auto &participant = *it;
            participant->deliver(message);
            it = mParticipants.erase(it);
        }
    });
}

void NetworkServer::join(std::shared_ptr<NetworkParticipant> participant) {
    mParticipants.insert(std::move(participant));
}

void NetworkServer::leave(std::shared_ptr<NetworkParticipant> participant) {
    mParticipants.erase(std::move(participant));
}

const std::shared_ptr<TournamentStore> & NetworkServer::getTournament() const {
    return mTournament;
}

const SharedActionList & NetworkServer::getActionStack() const {
    return mActionStack;
}

void NetworkServer::deliverAction(std::shared_ptr<NetworkMessage> message, std::shared_ptr<NetworkParticipant> sender) {
    ClientActionId actionId;
    std::shared_ptr<Action> action;
    message->decodeAction(actionId, action);

    mActionStack.push_back(std::make_pair(actionId, action));
    mActionMap[actionId] = std::prev(mActionStack.end());

    if (mActionStack.size() > ACTION_STACK_MAX_SIZE) {
        mActionStack.front().second->redo(*mTournament);
        mActionMap.erase(mActionStack.front().first);
        mActionStack.pop_front();
    }

    emit actionReceived(actionId, action->freshClone());

    for (auto & participant : mParticipants) {
        if (participant == sender) {
            auto ackMessage = std::make_unique<NetworkMessage>();
            ackMessage->encodeActionAck(actionId);
            participant->deliver(std::move(ackMessage));
            continue;
        }

        participant->deliver(message);
    }
}

void NetworkServer::deliverUndo(std::shared_ptr<NetworkMessage> message, std::shared_ptr<NetworkParticipant> sender) {
    ClientActionId actionId;
    message->decodeUndo(actionId);

    auto it = mActionMap.find(actionId);
    if (it != mActionMap.end()) {
        mActionStack.erase(it->second);
        mActionMap.erase(it);

        emit undoReceived(actionId);

        std::shared_ptr<NetworkMessage> sharedMessage = std::move(message);

        for (auto & participant : mParticipants) {
            if (participant == sender) {
                auto ackMessage = std::make_unique<NetworkMessage>();
                ackMessage->encodeUndoAck(actionId);
                participant->deliver(std::move(ackMessage));
                continue;
            }

            participant->deliver(sharedMessage);
        }
    }
}

void NetworkServer::deliver(std::shared_ptr<NetworkMessage> message) {
    for (auto & participant : mParticipants)
        participant->deliver(message);
}
