#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers
#include <map>
#include <set>
#include <queue>
#include <QObject>

#include "core/constants/actions.hpp"
#include "core/core.hpp"
#include "ui/network/network_interface.hpp"
#include "ui/store_managers/sync_payload.hpp"

class NetworkConnection;
class NetworkMessage;
class NetworkParticipant;

class NetworkServer : public NetworkInterface {
    Q_OBJECT
public:
    NetworkServer(boost::asio::io_context &context);

    void postSync(std::unique_ptr<TournamentStore> tournament) override;
    void postAction(ClientActionId actionId, std::unique_ptr<Action> action) override;
    void postUndo(ClientActionId actionId) override;

    void postStart(size_t port);
    void postShutdown();

    void stop() override;

signals:
    void serverStopped();
    void serverStarted();

private:
    void join(std::shared_ptr<NetworkParticipant> participant);
    void leave(std::shared_ptr<NetworkParticipant> participant);
    void deliverAction(std::shared_ptr<NetworkMessage> message, std::shared_ptr<NetworkParticipant> participant);
    void deliverUndo(std::shared_ptr<NetworkMessage> message, std::shared_ptr<NetworkParticipant> participant);
    void deliver(std::shared_ptr<NetworkMessage> message);

    const std::shared_ptr<TournamentStore> & getTournament() const;
    const SharedActionList & getActionStack() const;

protected:
    void postQuit();
    void accept();
private:
    boost::asio::io_context &mContext;
    boost::asio::ip::tcp::endpoint mEndpoint;
    boost::asio::ip::tcp::acceptor mAcceptor;
    std::unordered_set<std::shared_ptr<NetworkParticipant>> mParticipants;
    std::shared_ptr<TournamentStore> mTournament; // Tournament always kept behind the tournament in the UI thread
    SharedActionList mActionStack;
    std::unordered_map<ClientActionId, SharedActionList::iterator> mActionMap;

    friend class NetworkParticipant;
};

