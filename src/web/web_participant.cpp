#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/buffer.hpp>

#include "core/log.hpp"
#include "core/network/network_connection.hpp"
#include "web/json_encoder.hpp"
#include "web/loaded_tournament.hpp"
#include "web/web_participant.hpp"
#include "web/web_server.hpp"

// TODO: Try to see if message size can be limited in async_read
// TODO: Send error messages on load failure, tournament not exists etc.

WebParticipant::WebParticipant(boost::asio::io_context &context, std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> connection, WebServer &server, Database &database)
    : mContext(context)
    , mStrand(mContext)
    , mConnection(std::move(connection))
    , mServer(server)
    , mDatabase(database)
    , mClosePosted(false)
{
    mConnection->text(true);
}

void WebParticipant::listen() {
    auto self = shared_from_this();
    mConnection->async_read(mBuffer, boost::asio::bind_executor(mStrand, [this, self](boost::beast::error_code ec, std::size_t bytes_transferred) {
        if (mClosePosted)
            return;

        if (ec) {
            forceClose();
            return;
        }

        if (!parseMessage(boost::beast::buffers_to_string(mBuffer.data()))) {
            log_warning().msg("Failed parsing web message");
            // forceClose();
        }

        mBuffer.consume(bytes_transferred);

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
        if ('A' <= c && c <= 'Z')
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

    if (parts[0] == "subscribeTournament") {
        if (parts.size() != 2)
            return false;
        return subscribeTournament(parts[1]);
    }

    if (parts[0] == "subscribeCategory") {
        if (parts.size() != 2)
            return false;
        return subscribeCategory(parts[1]);
    }

    if (parts[0] == "subscribePlayer") {
        if (parts.size() != 2)
            return false;
        return subscribePlayer(parts[1]);
    }

    if (parts[0] == "subscribeTatami") {
        if (parts.size() != 2)
            return false;
        return subscribeTatami(parts[1]);
    }

    if (parts[0] == "listTournaments") {
        if (parts.size() != 1)
            return false;
        return listTournaments();
    }

    if (parts[0] == "clock") {
        if (parts.size() != 1)
            return false;
        return clock();
    }


    return false;
}

bool WebParticipant::subscribeTournament(const std::string &webName) {
    auto self = shared_from_this();
    mServer.getTournament(webName, boost::asio::bind_executor(mStrand, [this, self](std::shared_ptr<LoadedTournament> tournament) {
        if (mClosePosted)
            return;

        if (tournament == nullptr) {
            JsonEncoder encoder;
            deliver(encoder.encodeTournamentSubscriptionFailMessage());
            return;
        }

        tournament->addParticipant(shared_from_this());
        mTournament = std::move(tournament);
    }));

    return true;
}

void WebParticipant::asyncClose(CloseCallback callback) {
    // Send close frame
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, callback](){
        mClosePosted = true;
        mConnection->async_close(boost::beast::websocket::close_code::service_restart, [this, self, callback](boost::system::error_code ec) {
            if (mTournament != nullptr) {
                mTournament->eraseParticipant(shared_from_this());
                mTournament.reset();
            }

            if (mConnection)
                mConnection.reset();

            mServer.leave(shared_from_this(), callback);
        });
    });
}

void WebParticipant::forceClose() {
    if (mTournament != nullptr) {
        mTournament->eraseParticipant(shared_from_this());
        mTournament.reset();
    }

    if (mConnection)
        mConnection.reset();
    mServer.leave(shared_from_this(), []() {});
}

void WebParticipant::deliver(std::shared_ptr<JsonBuffer> message) {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, message, self](){
        if (mClosePosted)
            return;

        bool writeInProgress = !mWriteQueue.empty();
        mWriteQueue.push(std::move(message));

        if (!writeInProgress)
            write();
    });
}

void WebParticipant::write() {
    auto self = shared_from_this();
    const auto &message = mWriteQueue.front();
    mConnection->async_write(message->getBuffer(), boost::asio::bind_executor(mStrand, [this, self](boost::beast::error_code ec, std::size_t bytes_transferred) {
        if (mClosePosted)
            return;

        if (ec) {
            forceClose();
            return;
        }

        mWriteQueue.pop();

        if (!mWriteQueue.empty())
            write();
    }));
}

bool WebParticipant::subscribeCategory(const std::string &str) {
    try {
        if (mTournament == nullptr)
            return false;
        CategoryId id(str);
        mTournament->subscribeCategory(shared_from_this(), id);
    }
    catch (const std::exception &e) {
        return false;
    }

    return true;
}

bool WebParticipant::subscribeTatami(const std::string &str) {
    try {
        unsigned int index = std::stoul(str);
        mTournament->subscribeTatami(shared_from_this(), index);
    }
    catch (const std::exception &e) {
        return false;
    }

    return true;
}

bool WebParticipant::subscribePlayer(const std::string &str) {
    try {
        if (mTournament == nullptr)
            return false;
        PlayerId id(str);
        mTournament->subscribePlayer(shared_from_this(), id);
    }
    catch (const std::exception &e) {
        return false;
    }

    return true;
}

bool WebParticipant::listTournaments() {
    auto self = shared_from_this();
    mDatabase.asyncListTournaments(boost::asio::bind_executor(mStrand, [this, self](bool success, std::vector<TournamentListing> tournament) {
        if (mClosePosted)
            return;

        JsonEncoder encoder;

        if (!success) {
            deliver(encoder.encodeTournamentListingFailMessage());
            return;
        }

        deliver(encoder.encodeTournamentListingMessage(tournament));
    }));

    return true;
}

bool WebParticipant::clock() {
    auto t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    JsonEncoder encoder;
    deliver(encoder.encodeClockMessage(t));

    return true;
}

