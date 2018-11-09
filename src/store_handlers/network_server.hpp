#pragma once

#include <QObject>
#include <QThread>

// TODO: Do not include boost convenience headers
#include <boost/asio.hpp>
#include <set>
#include <queue>
#include <memory>

#include "core.hpp"
#include "store_handlers/network_message.hpp"
#include "store_handlers/network_connection.hpp"

class NetworkServer;

class NetworkParticipant : public std::enable_shared_from_this<NetworkParticipant> {
public:
    NetworkParticipant(std::shared_ptr<NetworkConnection> connection, NetworkServer *server)
        : mConnection(std::move(connection))
        , mServer(server)
        , mReadMessage(std::make_unique<NetworkMessage>())
        , mIsSyncing(true)
    {}

    void start();
    void deliver(std::shared_ptr<NetworkMessage> message);

    void setIsSyncing(bool value) {
        mIsSyncing = value;
    }

    bool isSyncing() const {
        return mIsSyncing;
    }
private:
    void readMessage();
    void writeMessage();

    std::shared_ptr<NetworkConnection> mConnection;
    NetworkServer *mServer;
    std::queue<std::shared_ptr<NetworkMessage>> mMessageQueue;
    std::unique_ptr<NetworkMessage> mReadMessage;
    bool mIsSyncing;
};

class NetworkServer : public QThread {
    Q_OBJECT
public:
    static const size_t ACTION_STACK_MAX_SIZE = 200; // TODO: Figure out if this is sufficient

    NetworkServer(int port);

    void postSync(std::unique_ptr<TournamentStore> tournament) {
        std::shared_ptr<TournamentStore> ptr = std::move(tournament);
        mContext.post([this, ptr]() {
            mTournament = std::move(ptr);
            mActionStack.clear();

            auto message = std::make_shared<NetworkMessage>();
            message->encodeSync(mTournament, mActionStack);

            for (auto & participant : mParticipants) {
                participant->setIsSyncing(true);
                participant->deliver(message);
            }
        });
    }

    void postAction(ActionId id, std::shared_ptr<Action> action) {
        mContext.post([this, id, action]() {
            mActionStack.push_back(std::make_pair(id, action));

            if (mActionStack.size() > ACTION_STACK_MAX_SIZE) {
                mActionStack.front().second->redo(*mTournament);
                mActionStack.pop_front();
            }

            auto message = std::make_shared<NetworkMessage>();
            message->encodeAction(id, action);

            for (auto & participant : mParticipants)
                participant->deliver(message);

            emit actionConfirmReceived(id);
        });
    }

    void postUndo(ActionId) {

    }

    void postQuit() {
        mContext.post([this]() {
            mAcceptor.close();

            auto message = std::make_shared<NetworkMessage>();
            message->encodeQuit();

            for (auto & participant : mParticipants) {
                participant->deliver(message);
                leave(participant);
            }
        });
    }

    void join(std::shared_ptr<NetworkParticipant> participant) {
        mParticipants.insert(participant);
    }

    void leave(std::shared_ptr<NetworkParticipant> participant) {
        mParticipants.erase(participant);
    }

    void deliver(std::shared_ptr<NetworkParticipant> sender, std::shared_ptr<NetworkMessage> message) {
        // TODO: emit signal
        for (auto & participant : mParticipants) {
            if (participant == sender)
                continue;

            participant->deliver(message);
        }
    }

    const std::shared_ptr<TournamentStore> & getTournament() const {
        return mTournament;
    }

    const std::list<std::pair<ActionId, std::shared_ptr<Action>>> & getActionStack() const {
        return mActionStack;
    }

signals:
    void actionReceived(ActionId actionId, std::shared_ptr<Action> action);
    void actionConfirmReceived(ActionId actionId);
    void syncConfirmed();

protected:
    void run() override;

    void accept();
private:
    boost::asio::io_context mContext;
    boost::asio::ip::tcp::endpoint mEndpoint;
    boost::asio::ip::tcp::acceptor mAcceptor;
    std::unordered_set<std::shared_ptr<NetworkParticipant>> mParticipants;
    std::shared_ptr<TournamentStore> mTournament; // Tournament always kept behind the tournament in the UI thread
    std::list<std::pair<ActionId, std::shared_ptr<Action>>> mActionStack;
};

