#include <lz4.h>

#include "core/log.hpp"
#include "core/network/network_message.hpp"
#include "core/serializables.hpp"
#include "core/version.hpp"
#include "core/web/web_types.hpp"

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

    compressed.resize(compressedSize);

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

void NetworkMessage::encodeHeader() {
    std::ostringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);

    size_t bodyLength = mBody.size();
    archive(mType, mUncompressedSize, bodyLength);

    mHeader = stream.str();
    assert(mHeader.size() == HEADER_LENGTH);
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
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(ApplicationVersion::current());

    encodeHeader();
}

bool NetworkMessage::decodeHandshake(ApplicationVersion &version) {
    return deserializeAndCompress(mUncompressedSize, mBody, version);
}

void NetworkMessage::encodeSyncAck() {
    mType = Type::SYNC_ACK;
    mBody.clear();
    mUncompressedSize = 0;
    encodeHeader();
}

void NetworkMessage::encodeSync(const TournamentStore & tournament, const NetworkMessage::SharedActionList &actionStack) {
    mType = Type::SYNC;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(tournament, actionStack);

    encodeHeader();
}

bool NetworkMessage::decodeSync(TournamentStore & tournament, NetworkMessage::SharedActionList &actionStack) {
    return deserializeAndCompress(mUncompressedSize, mBody, tournament, actionStack);
}

void NetworkMessage::encodeAction(const ClientActionId &actionId, const std::shared_ptr<Action> &action) {
    mType = Type::ACTION;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(actionId, action);

    encodeHeader();
}

bool NetworkMessage::decodeAction(ClientActionId &actionId, std::shared_ptr<Action> &action) {
    return deserializeAndCompress(mUncompressedSize, mBody, actionId, action);
}


void NetworkMessage::encodeActionAck(const ClientActionId &actionId) {
    mType = Type::ACTION_ACK;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(actionId);

    encodeHeader();
}

bool NetworkMessage::decodeActionAck(ClientActionId &actionId) {
    return deserializeAndCompress(mUncompressedSize, mBody, actionId);
}

void NetworkMessage::encodeUndoAck(const ClientActionId &actionId) {
    mType = Type::UNDO_ACK;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(actionId);

    encodeHeader();
}

bool NetworkMessage::decodeUndoAck(ClientActionId &actionId) {
    return deserializeAndCompress(mUncompressedSize, mBody, actionId);
}

void NetworkMessage::encodeUndo(const ClientActionId &actionId) {
    mType = Type::UNDO;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(actionId);

    encodeHeader();
}

bool NetworkMessage::decodeUndo(ClientActionId &actionId) {
    return deserializeAndCompress(mUncompressedSize, mBody, actionId);
}

void NetworkMessage::encodeQuit() {
    mType = Type::QUIT;
    mBody.clear();
    mUncompressedSize = 0;

    encodeHeader();
}

void NetworkMessage::encodeRequestWebToken(const std::string &email, const std::string &password) {
    mType = Type::REQUEST_WEB_TOKEN;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(email, password);

    encodeHeader();
}

bool NetworkMessage::decodeRequestWebToken(std::string &email, std::string &password) {
    return deserializeAndCompress(mUncompressedSize, mBody, email, password);
}

void NetworkMessage::encodeRequestWebTokenResponse(WebTokenRequestResponse response, const std::optional<WebToken> &token) {
    mType = Type::REQUEST_WEB_TOKEN_RESPONSE;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(response, token);

    encodeHeader();

}

bool NetworkMessage::decodeRequestWebTokenResponse(WebTokenRequestResponse &response, std::optional<WebToken> &token) {
    return deserializeAndCompress(mUncompressedSize, mBody, response, token);
}

void NetworkMessage::encodeValidateWebToken(const std::string &email, const WebToken &token) {
    mType = Type::VALIDATE_WEB_TOKEN;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(email, token);

    encodeHeader();
}

bool NetworkMessage::decodeValidateWebToken(std::string &email, WebToken &token) {
    return deserializeAndCompress(mUncompressedSize, mBody, email, token);
}

void NetworkMessage::encodeValidateWebTokenResponse(WebTokenValidationResponse response) {
    mType = Type::VALIDATE_WEB_TOKEN_RESPONSE;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(response);

    encodeHeader();
}

bool NetworkMessage::decodeValidateWebTokenResponse(WebTokenValidationResponse &response) {
    return deserializeAndCompress(mUncompressedSize, mBody, response);
}

void NetworkMessage::encodeRegisterWebName(const TournamentId &id, const std::string &webName) {
    mType = Type::REGISTER_WEB_NAME;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(id, webName);

    encodeHeader();
}

bool NetworkMessage::decodeRegisterWebName(TournamentId &id, std::string &webName) {
    return deserializeAndCompress(mUncompressedSize, mBody, id, webName);
}

void NetworkMessage::encodeRegisterWebNameResponse(const WebNameRegistrationResponse &response) {
    mType = Type::REGISTER_WEB_NAME_RESPONSE;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(response);

    encodeHeader();

}

bool NetworkMessage::decodeRegisterWebNameResponse(WebNameRegistrationResponse &response) {
    return deserializeAndCompress(mUncompressedSize, mBody, response);
}

void NetworkMessage::encodeCheckWebName(const TournamentId &id, const std::string &webName) {
    mType = Type::CHECK_WEB_NAME;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(id, webName);

    encodeHeader();
}

bool NetworkMessage::decodeCheckWebName(TournamentId &id, std::string &webName) {
    return deserializeAndCompress(mUncompressedSize, mBody, id, webName);
}

void NetworkMessage::encodeCheckWebNameResponse(const WebNameCheckResponse &response) {
    mType = Type::CHECK_WEB_NAME_RESPONSE;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(response);

    encodeHeader();
}

bool NetworkMessage::decodeCheckWebNameResponse(WebNameCheckResponse &response) {
    return deserializeAndCompress(mUncompressedSize, mBody, response);
}

std::ostream &operator<<(std::ostream &o, NetworkMessage::Type type) {
    if (type == NetworkMessage::Type::HANDSHAKE)
        return o << "HANDSHAKE";
    if (type == NetworkMessage::Type::SYNC)
        return o << "SYNC";
    if (type == NetworkMessage::Type::SYNC_ACK)
        return o << "SYNC_ACK";
    if (type == NetworkMessage::Type::ACTION)
        return o << "ACTION";
    if (type == NetworkMessage::Type::ACTION_ACK)
        return o << "ACTION_ACK";
    if (type == NetworkMessage::Type::UNDO)
        return o << "UNDO";
    if (type == NetworkMessage::Type::UNDO_ACK)
        return o << "UNDO_ACK";
    if (type == NetworkMessage::Type::QUIT)
        return o << "QUIT";
    if (type == NetworkMessage::Type::REQUEST_WEB_TOKEN)
        return o << "REQUEST_WEB_TOKEN";
    if (type == NetworkMessage::Type::REQUEST_WEB_TOKEN_RESPONSE)
        return o << "REQUEST_WEB_TOKEN_RESPONSE";
    if (type == NetworkMessage::Type::VALIDATE_WEB_TOKEN)
        return o << "VALIDATE_WEB_TOKEN";
    if (type == NetworkMessage::Type::VALIDATE_WEB_TOKEN_RESPONSE)
        return o << "VALIDATE_WEB_TOKEN_RESPONSE";
    if (type == NetworkMessage::Type::REGISTER_WEB_NAME)
        return o << "REGISTER_WEB_NAME";
    if (type == NetworkMessage::Type::REGISTER_WEB_NAME_RESPONSE)
        return o << "REGISTER_WEB_NAME_RESPONSE";
    if (type == NetworkMessage::Type::CHECK_WEB_NAME)
        return o << "CHECK_WEB_NAME";
    if (type == NetworkMessage::Type::CHECK_WEB_NAME_RESPONSE)
        return o << "CHECK_WEB_NAME_RESPONSE";
    if (type == NetworkMessage::Type::CLOCK_SYNC)
        return o << "CLOCK_SYNC";
    if (type == NetworkMessage::Type::CLOCK_SYNC_REQUEST)
        return o << "CLOCK_SYNC_REQUEST";
    return o << "INVALID";
}

void NetworkMessage::encodeClockSync(const std::chrono::milliseconds &time) {
    mType = Type::CLOCK_SYNC;
    std::tie(mBody, mUncompressedSize) = serializeAndCompress(time);

    encodeHeader();
}

bool NetworkMessage::decodeClockSync(std::chrono::milliseconds &time) {
    return deserializeAndCompress(mUncompressedSize, mBody, time);
}

void NetworkMessage::encodeClockSyncRequest() {
    mType = Type::QUIT;
    mBody.clear();
    mUncompressedSize = 0;

    encodeHeader();
}

