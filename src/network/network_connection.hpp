#pragma once

#include <boost/asio.hpp>

#include "core.hpp"
#include "version.hpp"
#include "network/network_message.hpp"

class NetworkConnection {
public:
    NetworkConnection(boost::asio::ip::tcp::socket socket)
        : mSocket(std::move(socket))
    {}

    template <typename Handler>
    void asyncJoin(Handler handler) {
        // Read handshake message and then send one back
        auto responseMessage = std::make_shared<NetworkMessage>();

        asyncRead(*responseMessage, [this, responseMessage, handler](boost::system::error_code ec) {
            if (ec) {
                handler(ec);
                return;
            }

            std::optional<ApplicationVersion> version;
            if (responseMessage->getType() != NetworkMessage::Type::HANDSHAKE || !(version = responseMessage->decodeHandshake())) {
                handler(boost::system::errc::make_error_code(boost::system::errc::protocol_error));
                return;
            }

            if (!version->compatible(ApplicationVersion::current())) {
                handler(boost::system::errc::make_error_code(boost::system::errc::protocol_not_supported));
                return;
            }

            auto handshakeMessage = std::make_shared<NetworkMessage>();
            handshakeMessage->encodeHandshake();

            asyncWrite(*handshakeMessage, [this, handshakeMessage, handler](boost::system::error_code ec) {
                if (ec) {
                    handler(ec);
                    return;
                }

                handler(boost::system::error_code()); // no error
            });
        });
    }

    template <typename Handler>
    void asyncAccept(Handler handler) {
        // Send handshake message and then read one
        auto handshakeMessage = std::make_shared<NetworkMessage>();
        handshakeMessage->encodeHandshake();

        asyncWrite(*handshakeMessage, [this, handshakeMessage, handler](boost::system::error_code ec) {
            if (ec) {
                handler(ec);
                return;
            }

            auto responseMessage = std::make_shared<NetworkMessage>();

            asyncRead(*responseMessage, [this, responseMessage, handler](boost::system::error_code ec) {
                if (ec) {
                    handler(ec);
                    return;
                }

                std::optional<ApplicationVersion> version;
                if (responseMessage->getType() != NetworkMessage::Type::HANDSHAKE || !(version = responseMessage->decodeHandshake())) {
                    handler(boost::system::errc::make_error_code(boost::system::errc::protocol_error));
                    return;
                }

                if (!version->compatible(ApplicationVersion::current())) {
                    handler(boost::system::errc::make_error_code(boost::system::errc::protocol_not_supported));
                    return;
                }

                handler(boost::system::error_code()); // no error
            });
        });
    }

    template <typename Handler>
    void asyncRead(NetworkMessage &message, Handler handler) {
        readHeader(message, handler);
    }

    template <typename Handler>
    void asyncWrite(NetworkMessage &message, Handler handler) {
        writeHeader(message, handler);
    }

private:
    template <typename Handler>
    void writeHeader(NetworkMessage &message, Handler handler) {
        // TODO: Consider writing header and body at the same time
        boost::asio::async_write(mSocket, message.headerBuffer(),
            [this, handler, &message](boost::system::error_code ec, size_t length) {
                if (ec) {
                    handler(ec);
                }
                else {
                    writeBody(message, handler);
                }
            }
        );
    }

    template <typename Handler>
    void writeBody(NetworkMessage &message, Handler handler) {
        if (message.bodySize() == 0) {
            // no body to write
            handler(boost::system::error_code());
            return;
        }

        boost::asio::async_write(mSocket, message.bodyBuffer(),
            [this, handler](boost::system::error_code ec, size_t length) {
                handler(ec);
            }
        );
    }

    template <typename Handler>
    void readHeader(NetworkMessage &message, Handler handler) {
        boost::asio::async_read(mSocket, message.headerBuffer(),
            [this, handler, &message](boost::system::error_code ec, size_t length) {
                if (ec) {
                    handler(ec);
                    return;
                }

                if (!message.decodeHeader()) {
                    handler(boost::system::errc::make_error_code(boost::system::errc::protocol_error));
                    return;
                }

                readBody(message, handler);
            }
        );
    }

    template <typename Handler>
    void readBody(NetworkMessage &message, Handler handler) {
        boost::asio::async_read(mSocket, message.bodyBuffer(),
            [this, handler](boost::system::error_code ec, size_t length) {
                handler(ec);
            }
        );
    }

private:
    boost::asio::ip::tcp::socket mSocket;
};
