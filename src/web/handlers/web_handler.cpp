#include <boost/asio/bind_executor.hpp>
#include <boost/beast.hpp>
#include <memory>

#include "core/logger.hpp"
#include "web/handlers/web_handler.hpp"
#include "web/handlers/web_handler_session.hpp"

WebHandler::WebHandler(boost::asio::io_context &context, Logger &logger, const Config &config)
    : mContext(context)
    , mLogger(logger)
    , mStrand(mContext)
    , mEndpoint(boost::asio::ip::tcp::v4(), config.webPort)
    , mAcceptor(mContext, mEndpoint)
{}

void WebHandler::async_listen() {
    mAcceptor.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (ec) {
            if (ec.value() != boost::system::errc::operation_canceled && ec.value() != boost::system::errc::bad_file_descriptor) {
                mLogger.warn("Unable to accept Web TCP", LoggerField(ec));
            }
        }
        else {
            const std::string address = socket.remote_endpoint().address().to_string();
            // We use a raw pointer due to difficulties passing smart pointers through asio
            auto websocket = new boost::beast::websocket::stream<boost::asio::ip::tcp::socket>(std::move(socket));
            websocket->async_accept(boost::asio::bind_executor(mStrand, [this, websocket, address](boost::beast::error_code ec) {
                if (ec) {
                    if (ec.value() != boost::system::errc::operation_canceled && ec.value() != boost::system::errc::bad_file_descriptor) {
                        mLogger.warn("Unable to accept Websocket", LoggerField(ec), LoggerField("address", address));
                    }
                    return;
                }

                mLogger.info("Accepted websocket request", LoggerField("address", address));

                auto connection_ptr = std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>(websocket);
                auto session = std::make_shared<WebHandlerSession>(mContext, mLogger, std::move(connection_ptr), *this);
                session->async_listen();
                mSessions.push_back(std::move(session));
            }));
        }

        if (mAcceptor.is_open())
            async_listen();
    });
}

void WebHandler::async_close() {
    mAcceptor.close();

    boost::asio::post(boost::asio::bind_executor(mStrand, [this]() {
        for (auto &session : mSessions) {
            session->async_close();
        }
    }));
}
