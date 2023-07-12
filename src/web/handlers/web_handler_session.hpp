#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>

class WebHandler;

// WebHandlerSession represents a stateful Websocket session. Sessions are
// created whenever a new Websocket connection is established.
class WebHandlerSession {
public:
    WebHandlerSession(boost::asio::io_context &context, std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> socket, WebHandler &web_handler);
    void async_listen();
    void async_close();

private:
    boost::asio::io_context &mContext;
    std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> mSocket;
    WebHandler &mWebHandler;
};
