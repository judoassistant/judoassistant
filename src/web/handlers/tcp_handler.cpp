#include <boost/asio/bind_executor.hpp>

#include "core/logger.hpp"
#include "core/network/network_connection.hpp"
#include "core/network/plain_socket.hpp"
#include "web/config/config.hpp"
#include "web/controllers/tournament_controller.hpp"
#include "web/handlers/tcp_handler.hpp"
#include "web/handlers/tcp_handler_session.hpp"

TCPHandler::TCPHandler(boost::asio::io_context &context, Logger &logger, const Config &config, TournamentController &tournamentController)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
    , mTournamentController(tournamentController)
    , mEndpoint(boost::asio::ip::tcp::v4(), config.tcpPort)
    , mAcceptor(mContext, mEndpoint)
{}

void TCPHandler::asyncListen() {
    mAcceptor.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (ec) {
            if (ec.value() != boost::system::errc::operation_canceled && ec.value() != boost::system::errc::bad_file_descriptor)
                mLogger.warn("Unable to accept TCP", LoggerField(ec));
        }
        else {
            const std::string address = socket.remote_endpoint().address().to_string();

            // We use a raw pointer due to difficulties passing smart pointers through asio
            auto connection = new NetworkConnection(std::make_unique<PlainSocket>(mContext, std::move(socket)));
            connection->asyncAccept(boost::asio::bind_executor(mStrand, [this, connection, address](boost::system::error_code ec) {
                auto connection_ptr = std::unique_ptr<NetworkConnection>(connection);

                if (ec) {
                    mLogger.warn("Unable to accept Judoassistant connection", LoggerField(ec), LoggerField("address", address));
                    return;
                }

                auto session = std::make_shared<TCPHandlerSession>(mContext, mLogger, *this, std::move(connection_ptr));
                session->asyncListen();
                mSessions.push_back(std::move(session));
            }));
        }

        if (mAcceptor.is_open()) {
            asyncListen();
        }
    });

}

void TCPHandler::asyncClose() {
    boost::asio::post(boost::asio::bind_executor(mStrand, [this]() {
        mAcceptor.close();

        for (auto &session : mSessions) {
            session->asyncClose();
        }
    }));
}
