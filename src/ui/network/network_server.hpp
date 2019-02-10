#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers
#include <map>
#include <set>
#include <queue>
#include <QObject>

#include "core/constants/actions.hpp"
#include "core/core.hpp"
#include "ui/network/network_interface.hpp"

class NetworkConnection;
class NetworkMessage;
class NetworkParticipant;

enum class NetworkServerState {
    STOPPED,
    STARTING,
    STARTED,
    STOPPING,
};

class NetworkServer : public NetworkInterface {
    Q_OBJECT
public:
    NetworkServer(boost::asio::io_context &context);

    void postSync(std::unique_ptr<TournamentStore> tournament) override;
    void postAction(ClientActionId actionId, std::unique_ptr<Action> action) override;
    void postUndo(ClientActionId actionId) override;

    void start(unsigned int port);
    void stop() override;

signals:
    void stateChanged(NetworkServerState state);

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
    NetworkServerState mState;
    boost::asio::io_context &mContext;
    std::optional<boost::asio::ip::tcp::endpoint> mEndpoint;
    std::optional<boost::asio::ip::tcp::acceptor> mAcceptor;
    std::unordered_set<std::shared_ptr<NetworkParticipant>> mParticipants;
    std::shared_ptr<TournamentStore> mTournament; // Tournament always kept behind the tournament in the UI thread
    SharedActionList mActionStack;
    std::unordered_map<ClientActionId, SharedActionList::iterator> mActionMap;

    friend class NetworkParticipant;
};

Q_DECLARE_METATYPE(NetworkServerState)

