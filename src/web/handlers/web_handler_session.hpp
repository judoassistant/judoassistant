#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <chrono>
#include <queue>

#include "core/id.hpp"
#include "web/controllers/tournament_controller.hpp"
#include "web/controllers/tournament_controller_session.hpp"
#include "web/mappers/websocket_json_mapper.hpp"
#include "core/logger.hpp"

class WebHandler;

// WebHandlerSession represents a stateful Websocket session. Sessions are
// created whenever a new Websocket connection is established.
class WebHandlerSession : public std::enable_shared_from_this<WebHandlerSession> {
public:
    WebHandlerSession(boost::asio::io_context &context, Logger &logger, std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> socket, WebHandler &webHandler, TournamentController &tournamentController);
    void asyncListen();
    void asyncClose();

    // Notifications from the controller layer
    void notifyTournamentChange(const WebTournamentStore &tournament, std::optional<CategoryId> categoryId, std::optional<PlayerId> playerId, std::optional<unsigned int> tatamiIndex, std::chrono::milliseconds clockDiff);
    void notifyTournamentSync(const WebTournamentStore &tournament, std::optional<CategoryId> categoryId, std::optional<PlayerId> playerId, std::optional<unsigned int> tatamiIndex, std::chrono::milliseconds clockDiff);
    void notifyCategorySubscription(const WebTournamentStore &tournament, const CategoryStore &category, std::chrono::milliseconds clockDiff);
    void notifyPlayerSubscription(const WebTournamentStore &tournament, const PlayerStore &player, std::chrono::milliseconds clockDiff);
    void notifyTatamiSubscription(const WebTournamentStore &tournament, size_t index, std::chrono::milliseconds clockDiff);

private:
    // close closes the current sessions and removes all references to the session from the handler and tournament.
    void close();

    // queueMessage pushes a message to the write queue. Once pushed, the
    // messages will be picked up from the queue in the order they were pushed.
    void queueMessage(const std::string &message);

    // queueMessage writes all messages in the queue in order that they were pushed.
    void writeMessageQueue();

    void handleClockCommand();
    void handleSubscribeTournamentCommand(const std::string &tournamentID);
    void handleSubscribeCategoryCommand();
    void handleSubscribePlayerCommand();
    void handleSubscribeTatamiCommand();
    void handleListTournamentsCommand();

    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Logger &mLogger;
    std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> mSocket;
    WebHandler &mWebHandler;
    WebsocketJSONMapper mMapper;
    TournamentController &mTournamentController;

    bool mIsClosed;
    std::queue<std::string> mWriteQueue;
    std::shared_ptr<TournamentControllerSession> mTournament;
};
