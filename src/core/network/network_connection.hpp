#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include "core/core.hpp"
#include "core/log.hpp"
#include "core/network/network_message.hpp"
#include "core/network/network_socket.hpp"
#include "core/version.hpp"

class NetworkMessage;

class NetworkConnection {
public:
    NetworkConnection(std::unique_ptr<NetworkSocket> socket)
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

            ApplicationVersion version;
            if (responseMessage->getType() != NetworkMessage::Type::HANDSHAKE || !responseMessage->decodeHandshake(version)) {
                handler(boost::system::errc::make_error_code(boost::system::errc::protocol_error));
                return;
            }

            if (!version.isCompatible(ApplicationVersion::current())) {
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

                ApplicationVersion version;
                if (responseMessage->getType() != NetworkMessage::Type::HANDSHAKE || !responseMessage->decodeHandshake(version)) {
                    handler(boost::system::errc::make_error_code(boost::system::errc::protocol_error));
                    return;
                }

                if (!ApplicationVersion::current().isCompatible(version)) {
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
        mSocket->asyncWrite(message.headerBuffer(), [this, handler, &message](boost::system::error_code ec, size_t length) {
            if (ec) {
                handler(ec);
            }
            else {
                writeBody(message, handler);
            }
        });
    }

    template <typename Handler>
    void writeBody(NetworkMessage &message, Handler handler) {
        if (message.bodySize() == 0) {
            // no body to write
            handler(boost::system::error_code());
            return;
        }

        mSocket->asyncWrite(message.bodyBuffer(), [this, handler](boost::system::error_code ec, size_t length) {
            handler(ec);
        });
    }

    template <typename Handler>
    void readHeader(NetworkMessage &message, Handler handler) {
        mSocket->asyncRead(message.headerBuffer(), [this, handler, &message](boost::system::error_code ec, size_t length) {
            if (ec) {
                handler(ec);
                return;
            }

            if (!message.decodeHeader()) {
                handler(boost::system::errc::make_error_code(boost::system::errc::protocol_error));
                return;
            }

            readBody(message, handler);
        });
    }

    template <typename Handler>
    void readBody(NetworkMessage &message, Handler handler) {
        mSocket->asyncRead(message.bodyBuffer(), [this, handler](boost::system::error_code ec, size_t length) {
            handler(ec);
        });
    }

private:
    std::unique_ptr<NetworkSocket> mSocket;
};

