#include "core/network/network_connection.hpp"
#include "web/database.hpp"
#include "web/loaded_tournament.hpp"
#include "web/web_participant.hpp"
#include "web/web_server.hpp"

WebParticipant::WebParticipant(boost::asio::io_context &context, std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> connection, WebServer &server, Database &database) {

}
