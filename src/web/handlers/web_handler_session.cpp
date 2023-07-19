#include "web/controllers/tournament_controller_session.hpp"
#include "web/handlers/web_handler.hpp"
#include "web/handlers/web_handler_session.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <memory>
#include <stdexcept>

constexpr int MAX_COMMAND_LENGTH = 200;

WebHandlerSession::WebHandlerSession(boost::asio::io_context &context, Logger &logger, std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> socket, WebHandler &webHandler, TournamentController &tournamentController)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
    , mSocket(std::move(socket))
    , mWebHandler(webHandler)
    , mTournamentController(tournamentController)
    , mIsClosed(false)
{
    mSocket->text(true);
}

void WebHandlerSession::asyncListen() {
    auto self = shared_from_this();
    auto buffer = std::make_shared<boost::beast::multi_buffer>();

    mSocket->async_read(*buffer, boost::asio::bind_executor(mStrand, [this, self, buffer](boost::beast::error_code ec, std::size_t bytes_transferred) {
        if (mIsClosed) {
            return;
        }
        if (ec) {
            close();
            return;
        }

        const auto message = boost::beast::buffers_to_string(buffer->data());
        buffer->consume(bytes_transferred);

        if (message.size() > MAX_COMMAND_LENGTH) {
            const auto message_substr = message.substr(0, MAX_COMMAND_LENGTH) + "...";
            mLogger.warn("Received too long websocket message", LoggerField("websocketMessage", message_substr));
            close();
            return;
        }

        std::vector<std::string> parts;
        boost::split(parts, message, boost::is_any_of(" "));
        if (message.empty() || parts.empty()) {
            mLogger.warn("Received empty websocket message");
            close();
            return;
        }

        mLogger.info("Received websocket message", LoggerField("websocketMessage", message));
        const auto command = parts[0];
        if (command == "subscribeTournament") {
            if (parts.size() != 2) {
                mLogger.warn("Received invalid number of arguments", LoggerField("websocketMessage", message));
                close();
                return;
            }

            // TODO: Limit valid tournament names
            handleSubscribeTournamentCommand(parts[1]);
        } else if (command == "subscribeCategory") {
            handleSubscribeCategoryCommand();
        } else if (command == "subscribePlayer") {
            handleSubscribePlayerCommand();
        } else if (command == "listTournaments") {
            handleListTournamentsCommand();
        } else if (command == "clock") {
            if (parts.size() != 1) {
                mLogger.warn("Received invalid number of arguments", LoggerField("websocketMessage", message));
                close();
                return;
            }

            handleClockCommand();
        } else {
            mLogger.warn("Received invalid websocket message", LoggerField("websocketMessage", message));
            close();
            return;
        }

        asyncListen();
    }));
}

void WebHandlerSession::handleClockCommand() {
    const auto unix_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    const auto resp = mMapper.mapClockMessage(unix_time);
    queueMessage(resp);
}

void WebHandlerSession::handleSubscribeTournamentCommand(const std::string &tournamentID) {
    auto self = shared_from_this();
    if (mTournament) {
        // Ignore if already subscribed to a tournament
        return;
    }

    mTournamentController.asyncSubscribeTournament(self, tournamentID, [this, self](boost::system::error_code ec, std::shared_ptr<TournamentControllerSession> tournamentSession){
        if (ec.value() == boost::system::errc::no_such_file_or_directory) {
            // TODO: Map not found
        }
    });
}

void WebHandlerSession::handleSubscribeCategoryCommand() {

}

void WebHandlerSession::handleSubscribePlayerCommand() {

}

void WebHandlerSession::handleSubscribeTatamiCommand() {

}

void WebHandlerSession::handleListTournamentsCommand() {

}

void WebHandlerSession::queueMessage(const std::string &message) {
    const auto queueWasEmpty = mWriteQueue.empty();
    mWriteQueue.push(message);

    if (!queueWasEmpty) {
        // The queue was non empty so a write was already in progress.
        return;
    }
    writeMessageQueue();
}

void WebHandlerSession::writeMessageQueue() {
    const auto buffer = boost::asio::buffer(mWriteQueue.front());
    auto self = shared_from_this();
    mSocket->async_write(buffer, boost::asio::bind_executor(mStrand, [this, self](boost::beast::error_code ec, std::size_t bytes_transferred) {
        if (mIsClosed) {
            return;
        }
        if (ec) {
            close();
            return;
        }

        mWriteQueue.pop();
        if (!mWriteQueue.empty()) {
            writeMessageQueue();
        }
    }));
}

void WebHandlerSession::asyncClose() {
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self](){
        if (mIsClosed) {
            return;
        }

        mSocket->async_close(boost::beast::websocket::close_code::service_restart, boost::asio::bind_executor(mStrand, [this, self](boost::system::error_code ec) {
            if (mIsClosed) {
                return;
            }

            close();
        }));
    });
}

void WebHandlerSession::close() {
    mIsClosed = true;
    mSocket.reset();
    // TODO: Remove from handler
}

void WebHandlerSession::notifyTournamentChange(const WebTournamentStore &tournament, std::optional<CategoryId> categoryId, std::optional<PlayerId> playerId, std::optional<unsigned int> tatamiIndex, std::chrono::milliseconds clockDiff) {
    const auto message = mMapper.mapChangeMessage(tournament, categoryId, playerId, tatamiIndex, clockDiff);
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, message]() {
        if (mIsClosed) {
            return;
        }

        queueMessage(message);
    });
}

void WebHandlerSession::notifyTournamentSync(const WebTournamentStore &tournament, std::optional<CategoryId> categoryId, std::optional<PlayerId> playerId, std::optional<unsigned int> tatamiIndex, std::chrono::milliseconds clockDiff) {
    const auto message = mMapper.mapSyncMessage(tournament, categoryId, playerId, tatamiIndex, clockDiff);
    auto self = shared_from_this();
    boost::asio::post(mStrand, [this, self, message]() {
        if (mIsClosed) {
            return;
        }

        queueMessage(message);
    });
}

void WebHandlerSession::notifyCategorySubscription(const WebTournamentStore &tournament, const CategoryStore &category, std::chrono::milliseconds clockDiff) {

}

void WebHandlerSession::notifyPlayerSubscription(const WebTournamentStore &tournament, const PlayerStore &player, std::chrono::milliseconds clockDiff) {

}

void WebHandlerSession::notifyTatamiSubscription(const WebTournamentStore &tournament, size_t index, std::chrono::milliseconds clockDiff) {

}
