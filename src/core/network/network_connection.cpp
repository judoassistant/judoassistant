#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include "core/network/network_connection.hpp"
#include "core/network/network_message.hpp"
#include "core/network/network_socket.hpp"
#include "core/version.hpp"

NetworkConnection::NetworkConnection(std::unique_ptr<NetworkSocket> socket)
    : mSocket(std::move(socket))
{}

void NetworkConnection::asyncJoin(JoinHandler handler) {
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

        asyncWrite(*handshakeMessage, [handshakeMessage, handler](boost::system::error_code ec) {
            if (ec) {
                handler(ec);
                return;
            }

            handler(boost::system::error_code()); // no error
        });
    });
}

void NetworkConnection::asyncAccept(AcceptHandler handler) {
    // Send handshake message and then read one
    auto handshakeMessage = std::make_shared<NetworkMessage>();
    handshakeMessage->encodeHandshake();

    asyncWrite(*handshakeMessage, [this, handshakeMessage, handler](boost::system::error_code ec) {
        if (ec) {
            handler(ec);
            return;
        }

        auto responseMessage = std::make_shared<NetworkMessage>();

        asyncRead(*responseMessage, [responseMessage, handler](boost::system::error_code ec) {
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

void NetworkConnection::asyncRead(NetworkMessage &message, ReadHandler handler) {
    readHeader(message, std::move(handler));
}

void NetworkConnection::asyncWrite(NetworkMessage &message, WriteHandler handler) {
    writeHeader(message, std::move(handler));
}

void NetworkConnection::writeHeader(NetworkMessage &message, WriteHandler handler) {
    // TODO: Consider writing header and body at the same time
    mSocket->asyncWrite(message.headerBuffer(), [this, handler, &message](boost::system::error_code ec, size_t length) {
        if (ec) {
            handler(ec);
        }
        else {
            writeBody(message, std::move(handler));
        }
    });
}

void NetworkConnection::writeBody(NetworkMessage &message, WriteHandler handler) {
    if (message.bodySize() == 0) {
        // no body to write
        handler(boost::system::error_code());
        return;
    }

    mSocket->asyncWrite(message.bodyBuffer(), [handler](boost::system::error_code ec, size_t length) {
        handler(ec);
    });
}

void NetworkConnection::readHeader(NetworkMessage &message, ReadHandler handler) {
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

void NetworkConnection::readBody(NetworkMessage &message, ReadHandler handler) {
    mSocket->asyncRead(message.bodyBuffer(), [handler](boost::system::error_code ec, size_t length) {
        handler(ec);
    });
}

