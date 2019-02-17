#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/buffer.hpp>

#include "core/network/network_connection.hpp"
#include "web/database.hpp"
#include "web/loaded_tournament.hpp"
#include "web/web_participant.hpp"
#include "web/web_server.hpp"

// TODO: Fix segfault on server SIGINT
// TODO: Try to see if message size can be limited in async_read

WebParticipant::WebParticipant(boost::asio::io_context &context, std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> connection, WebServer &server, Database &database)
    : mContext(context)
    , mStrand(context)
    , mConnection(std::move(connection))
    , mServer(server)
    , mDatabase(database)
{
    mConnection->text(true);
}

void WebParticipant::listen() {
    auto self = shared_from_this();
    mConnection->async_read(mBuffer, boost::asio::bind_executor(mStrand, [this, self](boost::beast::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            forceQuit();
            return;
        }

        if (!parseMessage(boost::beast::buffers_to_string(mBuffer.data()))) {
            forceQuit();
            return;
        }

        listen();
    }));
}

bool WebParticipant::validateMessage(const std::string &message) {
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

    std::vector<std::string> parts;
    boost::split(parts, message, boost::is_any_of(" "));

    if (parts[0] == "select-tournament") {
        if (parts.size() != 2)
            return false;
        selectTournament(parts[1]);
        return true;
    }

    if (parts[0] == "select-category") {
    }

    if (parts[0] == "select-player") {
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
    auto self = shared_from_this();
    mServer.getTournament(webName, boost::asio::bind_executor(mStrand, [this, self](std::shared_ptr<LoadedTournament> tournament) {
        tournament->addParticipant(shared_from_this());
        tournament->generateSyncJson(boost::asio::bind_executor(mStrand, [this, self](std::shared_ptr<rapidjson::StringBuffer> message) {
            deliver(std::move(message));
        }));
        mTournament = std::move(tournament);
    }));
}

void WebParticipant::quit() {
    // Since WebParticipants are read-only there is not risk of data corruption
    // when killing

    auto self = shared_from_this();
    boost::asio::dispatch(mStrand, [this, self]() {
        forceQuit();
    });
}

void WebParticipant::forceQuit() {
    if (mTournament != nullptr) {
        mTournament->eraseParticipant(shared_from_this());
        mTournament = nullptr;
    }

    mConnection.reset();
    mServer.leave(shared_from_this());
}

void WebParticipant::deliver(std::shared_ptr<rapidjson::StringBuffer> message) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, message, self](){
        bool writeInProgress = !mWriteQueue.empty();
        mWriteQueue.push(std::move(message));

        if (!writeInProgress)
            write();
    });
}

void WebParticipant::write() {
    auto self = shared_from_this();
    const auto &message = mWriteQueue.front();
    auto buffer = boost::asio::buffer(message->GetString(), message->GetSize());
    mConnection->async_write(buffer, boost::asio::bind_executor(mStrand, [this, self](boost::beast::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            forceQuit();
            return;
        }

        mWriteQueue.pop();

        if (!mWriteQueue.empty())
            write();
    }));
}

