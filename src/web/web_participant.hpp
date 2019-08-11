#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <queue>

class WebServer;
class LoadedTournament;
class NetworkConnection;
class JsonBuffer;

class WebParticipant : public std::enable_shared_from_this<WebParticipant> {
public:
    WebParticipant(boost::asio::io_context &context, std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> connection, WebServer &server);

    void quit();
    void listen();
    void deliver(std::shared_ptr<JsonBuffer> message);

private:
    void forceQuit();
    bool parseMessage(const std::string &message);
    bool validateMessage(const std::string &message);

    bool subscribeTournament(const std::string &webName);
    bool subscribeCategory(const std::string &id);
    bool subscribePlayer(const std::string &id);
    bool subscribeTatami(const std::string &index);
    bool listTournaments();

    void write();

    boost::asio::io_context& mContext;
    boost::asio::io_context::strand mStrand;
    std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> mConnection;
    boost::beast::multi_buffer mBuffer;

    WebServer &mServer;

    std::shared_ptr<LoadedTournament> mTournament;
    std::queue<std::shared_ptr<JsonBuffer>> mWriteQueue;
};

