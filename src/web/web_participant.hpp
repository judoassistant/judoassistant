#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

class Database;
class WebServer;
class LoadedTournament;
class NetworkConnection;

class WebParticipant {
public:
    WebParticipant(boost::asio::io_context &context, std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> connection, WebServer &server, Database &database);

private:
    // boost::asio::io_context& mContext;
    // boost::asio::io_context::strand mStrand;

    std::shared_ptr<LoadedTournament> mTournament;
};

