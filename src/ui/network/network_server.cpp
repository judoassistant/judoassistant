#include "core/log.hpp"
#include "core/network/network_connection.hpp"
#include "core/network/network_message.hpp"
#include "core/network/plain_socket.hpp"
#include "ui/network/network_participant.hpp"
#include "ui/network/network_server.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/web/web_client.hpp"

using boost::asio::ip::tcp;

NetworkServer::NetworkServer(boost::asio::io_context &context, WebClient &webClient)
    : mState(NetworkServerState::STOPPED)
    , mContext(context)
    , mTournament(std::make_shared<TournamentStore>())
    , mWebClient(webClient)
{
    qRegisterMetaType<NetworkServerState>();
}

void NetworkServer::start(unsigned int port) {
    mContext.post([this, port]() {
        if (mState != NetworkServerState::STOPPED) {
            log_warning().msg("Tried to call accept on already accepting server");
            return;
        }

        try {
            mEndpoint = boost::asio::ip::tcp::endpoint(tcp::v4(), port);
            mAcceptor = boost::asio::ip::tcp::acceptor(mContext, *mEndpoint);
        }
        catch (const std::exception &e) {
            emit startFailed();
            return;
        }

        accept();

        emit stateChanged(mState = NetworkServerState::STARTED);
    });
}

void NetworkServer::stop() {
    mContext.post([this]() {
        if (mState != NetworkServerState::STARTED)
            return;

        mAcceptor->close();

        auto message = std::make_shared<NetworkMessage>();
        message->encodeQuit();

        auto it = mParticipants.begin();
        while (it != mParticipants.end()) {
            auto &participant = *it;
            participant->deliver(message);
            it = mParticipants.erase(it);
        }

        emit stateChanged(mState = NetworkServerState::STOPPED);
    });
}

void NetworkServer::accept() {
    mAcceptor->async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (ec) {
            log_error().field("message", ec.message()).msg("Received error code in async_accept");
        }
        else {
            auto connection = std::make_shared<NetworkConnection>(std::make_unique<PlainSocket>(mContext, std::move(socket)));

            connection->asyncAccept([this, connection](boost::system::error_code ec) {
                if (ec) {
                    log_error().field("message", ec.message()).msg("Received error code in connection.asyncAccept");
                }
                else {
                    std::make_shared<NetworkParticipant>(std::move(connection), *this)->start();
                }
            });
        }

        if (mAcceptor->is_open())
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

        mWebClient.deliver(message);

        emit syncConfirmed();
    });
}

void NetworkServer::postAction(ClientActionId actionId, std::unique_ptr<Action> action) {
    std::shared_ptr<Action> sharedAction = std::move(action);
    mContext.post([this, actionId, sharedAction]() {
        mActionStack.push_back(std::make_pair(actionId, sharedAction));
        mActionMap[actionId] = std::prev(mActionStack.end());

        if (mActionStack.size() > MAX_ACTION_STACK_SIZE) {
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

    if (mActionStack.size() > MAX_ACTION_STACK_SIZE) {
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

    mWebClient.deliver(message);
}

void NetworkServer::deliver(std::shared_ptr<NetworkMessage> message) {
    for (auto & participant : mParticipants)
        participant->deliver(message);
    mWebClient.deliver(message);
}

