#include <boost/beast.hpp>
#include "web/handlers/web_handler.hpp"
#include "core/logger.hpp"

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
            auto connection = std::make_shared<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>(std::move(socket));
            connection->async_accept(boost::asio::bind_executor(mStrand, [this, connection](boost::beast::error_code ec) {
                if (ec) {
                    if (ec.value() != boost::system::errc::operation_canceled && ec.value() != boost::system::errc::bad_file_descriptor) {
                        mLogger.warn("Unable to accept Websocket", LoggerField(ec));
                    }
                    return;
                }

                log_info().msg("Accepted websocket request");
                // auto participant = std::make_shared<WebParticipant>(mContext, std::move(connection), *this, *mDatabase);
                // participant->listen();
                // mWebParticipants.insert(std::move(participant));
            }));
        }

        if (mAcceptor.is_open())
            async_listen();
    });
}

void WebHandler::close() {
    mAcceptor.close();
}
