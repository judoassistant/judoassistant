#pragma once

#include <boost/asio/buffer.hpp>
#include <list>
#include <sstream>
#include <variant>
#include <optional>

#include "core.hpp"
#include "id.hpp"
#include "web/web_types.hpp"

class ApplicationVersion;
class TournamentStore;
class Action;

class NetworkMessage {
public:
    static const size_t HEADER_LENGTH = 17; // cereal::PortableBinaryOutputArchive uses 1 + 8 + 4 + 4 bytes for the header

    enum class Type {
        HANDSHAKE, // The message contains the initial connection handshake
        SYNC, // The message contains the entire serialized tournament
        SYNC_ACK, // The message acknowledges the sync
        ACTION, // The message contains a serialized action
        ACTION_ACK, // The message acknowledges an action
        UNDO, // The message undoes a previously sent action
        UNDO_ACK, // The message acknowledges an undo
        QUIT, // The participant or server has quit

        // Messages used for communication with the web server
        REQUEST_WEB_TOKEN,
        REQUEST_WEB_TOKEN_RESPONSE,
        VALIDATE_WEB_TOKEN,
        VALIDATE_WEB_TOKEN_RESPONSE,
        REGISTER_WEB_NAME,
        REGISTER_WEB_NAME_RESPONSE,
        CHECK_WEB_NAME,
        CHECK_WEB_NAME_RESPONSE,
    };

    NetworkMessage();

    boost::asio::mutable_buffer headerBuffer();
    boost::asio::mutable_buffer bodyBuffer();

    bool decodeHeader();
    size_t bodySize() const;
    Type getType() const;

    void encodeHandshake();
    bool decodeHandshake(ApplicationVersion &version);

    typedef std::list<std::pair<ClientActionId, std::shared_ptr<Action>>> SharedActionList;
    void encodeSync(const TournamentStore & tournament, const SharedActionList &actionStack);
    bool decodeSync(TournamentStore & tournament, SharedActionList &actionStack);

    void encodeSyncAck();

    void encodeAction(const ClientActionId &actionId, const std::shared_ptr<Action> &action);
    bool decodeAction(ClientActionId &actionId, std::shared_ptr<Action> &action);

    void encodeActionAck(const ClientActionId &actionId);
    bool decodeActionAck(ClientActionId &actionId);

    void encodeQuit();

    void encodeUndo(const ClientActionId &actionId);
    bool decodeUndo(ClientActionId &actionId);

    void encodeUndoAck(const ClientActionId &actionId);
    bool decodeUndoAck(ClientActionId &actionId);

    void encodeRequestWebToken(const std::string &email, const std::string &password);
    bool decodeRequestWebToken(std::string &email, std::string &password);

    void encodeRequestWebTokenResponse(WebTokenRequestResponse response, const std::optional<WebToken> &token);
    bool decodeRequestWebTokenResponse(WebTokenRequestResponse &response, std::optional<WebToken> &token);

    void encodeValidateWebToken(const std::string &email, const WebToken &token);
    bool decodeValidateWebToken(std::string &email, WebToken &token);

    void encodeValidateWebTokenResponse(WebTokenValidationResponse response);
    bool decodeValidateWebTokenResponse(WebTokenValidationResponse &response);

    void encodeRegisterWebName(const TournamentId &id, const std::string &webName);
    bool decodeRegisterWebName(TournamentId &id, std::string &webName);

    void encodeRegisterWebNameResponse(const WebNameRegistrationResponse &response);
    bool decodeRegisterWebNameResponse(WebNameRegistrationResponse &response);

    void encodeCheckWebName(const TournamentId &id, const std::string &webName);
    bool decodeCheckWebName(TournamentId &id, std::string &webName);

    void encodeCheckWebNameResponse(const WebNameCheckResponse &response);
    bool decodeCheckWebNameResponse(WebNameCheckResponse  &response);

private:
    void encodeHeader();

    Type mType;
    int mUncompressedSize;
    std::string mHeader;
    std::string mBody;
};

