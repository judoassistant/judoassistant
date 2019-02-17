#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <rapidjson/stringbuffer.h>
#include <queue>

class Database;
class WebServer;
class LoadedTournament;
class NetworkConnection;

class WebParticipant : public std::enable_shared_from_this<WebParticipant> {
public:
    WebParticipant(boost::asio::io_context &context, std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> connection, WebServer &server, Database &database);

    void quit();
    void listen();

private:
    void forceQuit();
    bool parseMessage(const std::string &message);
    bool validateMessage(const std::string &message);

    void selectTournament(const std::string &webName);
    void listTournaments();

    void deliver(std::shared_ptr<rapidjson::StringBuffer> message);
    void write();

    boost::asio::io_context& mContext;
    boost::asio::io_context::strand mStrand;
    std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> mConnection;
    boost::beast::multi_buffer mBuffer;

    WebServer &mServer;
    Database &mDatabase;

    std::shared_ptr<LoadedTournament> mTournament;
    std::queue<std::shared_ptr<rapidjson::StringBuffer>> mWriteQueue;
};

