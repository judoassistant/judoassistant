#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio/bind_executor.hpp>

#include "core/network/network_connection.hpp"
#include "web/database.hpp"
#include "web/loaded_tournament.hpp"
#include "web/web_participant.hpp"
#include "web/web_server.hpp"

WebParticipant::WebParticipant(boost::asio::io_context &context, std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> connection, WebServer &server, Database &database)
    : mContext(context)
    , mStrand(context)
    , mConnection(std::move(connection))
    , mServer(server)
    , mDatabase(database)
{
    listen();
}

void WebParticipant::listen() {
    mConnection->async_read(mBuffer, boost::asio::bind_executor(mStrand, [this](boost::beast::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            log_debug().field("message", ec.message()).msg("WebParticipant: failed async_read");
            forceQuit();
            return;
        }

        if (!parseMessage(boost::beast::buffers_to_string(mBuffer.data()))) {
            log_debug().msg("WebParticipant: failed parsing message");
            forceQuit();
            return;
        }

        listen();
    }));
}

bool WebParticipant::validateMessage(const std::string &message) {
    // TODO: Try to check message size earlier
    if (message.empty() || message.size() > 200)
        return false;
    for (const char c : message) {
        // Only allow certain characters
        if ('a' <= c && c <= 'z')
            continue;
        if ('0' <= c && c <= '9')
            continue;
        if (c == '-')
            continue;
        if (c == ' ')
            continue;

        return false;
    }

    return true;
};

bool WebParticipant::parseMessage(const std::string &message) {
    if (!validateMessage(message))
        return false;

    log_debug().field("message", message).msg("Parsing message");
    std::vector<std::string> parts;
    boost::split(parts, message, boost::is_any_of(" "));

    if (parts[0] == "select-tournament") {
        if (parts.size() != 2)
            return false;
        selectTournament(parts[1]);
        return true;
    }

    if (parts[0] == "list-tournaments") {
        if (parts.size() != 1)
            return false;
        listTournaments();
        return true;
    }

    return false;
}

void WebParticipant::listTournaments() {
    log_debug().msg("Listing tournaments");
}

void WebParticipant::selectTournament(const std::string &webName) {
    log_debug().field("webName", webName).msg("Selecting tournament");
    mServer.getTournament(webName, [this](std::shared_ptr<LoadedTournament> tournament) {
        log_debug().field("isNull", tournament == nullptr).msg("Got tournament");
    });
}

void WebParticipant::quit() {
    // TODO: Implement
    mServer.leave(shared_from_this());
}

void WebParticipant::forceQuit() {
    // TODO: Implement
    mServer.leave(shared_from_this());
}

