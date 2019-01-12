#include "lz4.h"

#include "log.hpp"
#include "network/network_message.hpp"
#include "serializables.hpp"
#include "version.hpp"

// Perfect forwards args to cereal archive and compresses the result
template <typename... Args>
std::tuple<std::string, int> serializeAndCompress(Args&&... args) {
    std::string uncompressed;

    {
        std::ostringstream stream;
        cereal::PortableBinaryOutputArchive archive(stream);
        archive(std::forward<Args>(args)...);
        uncompressed = stream.str();
    }

    const int uncompressedSize = static_cast<int>(uncompressed.size());
    const int compressBound = LZ4_compressBound(uncompressedSize);

    std::string compressed;
    compressed.resize(compressBound);

    const int compressedSize = LZ4_compress_default(uncompressed.data(), compressed.data(), uncompressed.size(), compressBound);

    if (compressedSize <= 0) {
        log_error().field("return_value", compressedSize).msg("LZ4 compress failed");
        throw std::runtime_error("LZ4 compress failed");
    }

    return {std::move(compressed), uncompressedSize};
}

// Forwards args to cereal archive and decompress the result
template <typename... Args>
bool deserializeAndCompress(int uncompressedSize, const std::string &compressed, Args&&... args) {
    std::string uncompressed;
    uncompressed.resize(uncompressedSize);

    auto returnCode = LZ4_decompress_safe(compressed.data(), uncompressed.data(), compressed.size(), uncompressedSize);

    if (returnCode <= 0) {
        log_error().field("return_value", returnCode).msg("LZ4 decompress failed");
        return false;
    }

    try
    {
        std::istringstream stream(uncompressed);
        cereal::PortableBinaryInputArchive archive(stream);
        archive(args...);
    }
    catch (const std::exception &e) {
        log_error().field("what", e.what()).msg("Failed serialization of message");
        return false;
    }

    return true;
}

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
        archive(mType, mUncompressedSize, bodyLength);
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

void NetworkMessage::encodeSync(const TournamentStore & tournament, const NetworkMessage::SharedActionList &actionStack) {
    mType = Type::SYNC;

    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(tournament, actionStack);

    mBody = stream.str();
    encodeHeader();
}

bool NetworkMessage::decodeSync(TournamentStore & tournament, NetworkMessage::SharedActionList &actionStack) {
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

void NetworkMessage::encodeAction(const ClientActionId &actionId, const std::shared_ptr<Action> &action) {
    mType = Type::ACTION;

    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(actionId, action);

    mBody = stream.str();
    encodeHeader();
}

void NetworkMessage::encodeActionAck(const ClientActionId &actionId) {
    mType = Type::ACTION_ACK;

    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(actionId);

    mBody = stream.str();
    encodeHeader();
}

void NetworkMessage::encodeUndoAck(const ClientActionId &actionId) {
    mType = Type::UNDO_ACK;

    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(actionId);

    mBody = stream.str();
    encodeHeader();
}

bool NetworkMessage::decodeAction(ClientActionId &actionId, std::shared_ptr<Action> &action) {
    std::istringstream stream(mBody);
    cereal::PortableBinaryInputArchive archive(stream);

    try {
        archive(actionId, action);
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

void NetworkMessage::encodeUndo(const ClientActionId &actionId) {
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
    archive(mType, mUncompressedSize, bodyLength);

    mHeader = stream.str();
    assert(mHeader.size() == HEADER_LENGTH);
}

bool NetworkMessage::decodeUndo(ClientActionId &actionId) {
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

bool NetworkMessage::decodeUndoAck(ClientActionId &actionId) {
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

bool NetworkMessage::decodeActionAck(ClientActionId &actionId) {
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
