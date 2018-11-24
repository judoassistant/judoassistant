#include "network/network_message.hpp"
#include "serializables.hpp"
#include "version.hpp"

NetworkMessage::NetworkMessage() {
    mHeader.resize(HEADER_LENGTH);
}

boost::asio::mutable_buffer NetworkMessage::headerBuffer() {
    return boost::asio::buffer(mHeader, HEADER_LENGTH);
}

boost::asio::mutable_buffer NetworkMessage::bodyBuffer() {
    return boost::asio::buffer(mBody.data(), mBody.size());
}

size_t NetworkMessage::bodySize() const {
    return mBody.size();
}

bool NetworkMessage::decodeHeader() {
    try {
        std::istringstream stream(mHeader);
        cereal::PortableBinaryInputArchive archive(stream);

        size_t bodyLength;
        archive(bodyLength, mType);
        mBody.resize(bodyLength);
    }
    catch (const std::exception &e) {
        return false;
    }

    return true;
}

NetworkMessage::Type NetworkMessage::getType() const {
    return mType;
}

void NetworkMessage::encodeHandshake() {
    mType = Type::HANDSHAKE;

    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(ApplicationVersion::current());

    mBody = stream.str();
    log_debug().field("bodysize", mBody.size()).msg("Encoding handshake");
    encodeHeader();
}

bool NetworkMessage::decodeHandshake(ApplicationVersion &version) {
    std::istringstream stream(mBody);
    cereal::PortableBinaryInputArchive archive(stream);

    try {
        archive(version);
        return true;
    }
    catch (const std::exception &e) {
        return false;
    }
}

void NetworkMessage::encodeSyncAck() {
    mType = Type::SYNC_ACK;
    mBody.clear();
    encodeHeader();
}

void NetworkMessage::encodeSync(const TournamentStore & tournament, const NetworkMessage::ActionList &actionStack) {
    mType = Type::SYNC;

    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(tournament, actionStack);

    mBody = stream.str();
    encodeHeader();
}

bool NetworkMessage::decodeSync(TournamentStore & tournament, NetworkMessage::ActionList &actionStack) {
    std::istringstream stream(mBody);
    cereal::PortableBinaryInputArchive archive(stream);

    try {
        archive(tournament, actionStack);
    }
    catch (const std::exception &e) {
        return false;
    }

    return true;
}

void NetworkMessage::encodeAction(const ClientId &clientId, const ActionId &actionId, const std::shared_ptr<Action> &action) {
    mType = Type::ACTION;

    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(clientId, actionId, action);

    mBody = stream.str();
    encodeHeader();
}

void NetworkMessage::encodeActionAck(const ActionId &actionId) {
    mType = Type::ACTION_ACK;

    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(actionId);

    mBody = stream.str();
    encodeHeader();
}

void NetworkMessage::encodeUndoAck(const ActionId &actionId) {
    mType = Type::UNDO_ACK;

    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(actionId);

    mBody = stream.str();
    encodeHeader();
}

bool NetworkMessage::decodeAction(ClientId &clientId, ActionId &actionId, std::shared_ptr<Action> &action) {
    std::istringstream stream(mBody);
    cereal::PortableBinaryInputArchive archive(stream);

    try {
        archive(clientId, actionId, action);
        return true;
    }
    catch (const std::exception &e) {
        return false;
    }
}

void NetworkMessage::encodeQuit() {
    mType = Type::QUIT;
    mBody.clear();
    encodeHeader();
}

void NetworkMessage::encodeUndo(const ActionId &actionId) {
    mType = Type::UNDO;

    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(actionId);

    mBody = stream.str();
    encodeHeader();
}

void NetworkMessage::encodeHeader() {
    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);

    size_t bodyLength = mBody.size();
    archive(bodyLength, mType);

    mHeader = stream.str();
    assert(mHeader.size() == HEADER_LENGTH);
}

bool NetworkMessage::decodeUndo(ActionId &actionId) {
    std::istringstream stream(mBody);
    cereal::PortableBinaryInputArchive archive(stream);

    try {
        archive(actionId);
        return true;
    }
    catch (const std::exception &e) {
        return false;
    }
}

bool NetworkMessage::decodeUndoAck(ActionId &actionId) {
    std::istringstream stream(mBody);
    cereal::PortableBinaryInputArchive archive(stream);

    try {
        archive(actionId);
        return true;
    }
    catch (const std::exception &e) {
        return false;
    }
}

bool NetworkMessage::decodeActionAck(ActionId &actionId) {
    std::istringstream stream(mBody);
    cereal::PortableBinaryInputArchive archive(stream);

    try {
        archive(actionId);
        return true;
    }
    catch (const std::exception &e) {
        return false;
    }
}
