#pragma once

#include <chrono>
#include <memory>
#include <queue>
#include <boost/asio/io_context_strand.hpp>

#include "core/network/network_connection.hpp"
#include "core/network/network_message.hpp"
#include "web/controllers/tournament_controller.hpp"
#include "web/gateways/meta_service_gateway.hpp"

class TCPHandler;
class Logger;

// TCPHandlerSession represents a stateful TCP socket session. Sessions are
// created whenever a new TCP connection is established from the JudoAssistant
// main application.
class TCPHandlerSession : public std::enable_shared_from_this<TCPHandlerSession> {
public:
    TCPHandlerSession(boost::asio::io_context &context, Logger &logger, TCPHandler &tcpHandler, MetaServiceGateway &metaServiceGateway, TournamentController &tournamentController, std::unique_ptr<NetworkConnection> connection);

    void asyncClose();
    void asyncListen();

private:
    enum class State {
        INITIAL,
        AUTHENTICATED,
        TOURNAMENT_REGISTERED,
        TOURNAMENT_SYNCING,
        TOURNAMENT_LISTENING,
    };

    void close();
    void queueMessage(std::unique_ptr<NetworkMessage> message);

    void handleAuthentication();
    void handleTournamentRegistration();
    void handleTournamentClockSync();
    void handleTournamentSync();
    void handleTournamentListen();

    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Logger &mLogger;
    TCPHandler &mTCPHandler;
    MetaServiceGateway &mMetaServiceGateway;
    TournamentController &mTournamentController;
    std::unique_ptr<NetworkConnection> mConnection;

    State mState;
    bool mIsClosed;
    int mUserID;
    std::queue<std::shared_ptr<NetworkMessage>> mWriteQueue;
    std::chrono::milliseconds mClockDiff;
};
