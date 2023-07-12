#include "web/handlers/web_handler.hpp"
#include "web/handlers/web_handler_session.hpp"

WebHandlerSession::WebHandlerSession(boost::asio::io_context &context, std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> socket, WebHandler &web_handler)
    : mContext(context)
    , mSocket(std::move(socket))
    , mWebHandler(web_handler)
{
    mSocket->text(true);
}

void WebHandlerSession::async_listen() {

}

void WebHandlerSession::async_close() {

}
