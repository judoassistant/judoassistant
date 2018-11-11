#pragma once

#include <variant>
#include <sstream>
#include <boost/asio/buffer.hpp>

#include "core.hpp"
#include "id.hpp"
#include "actions/action.hpp"
#include "serializables.hpp"

class NetworkMessage {
public:
    static const size_t HEADER_LENGTH = 13; // cereal::PortableBinaryOutputArchive uses 1 + 8 + 4 bytes for the header

    enum class Type {
        HANDSHAKE, // The message contains the initial connection handshake
        SYNC, // The message contains the entire serialized tournament
        SYNC_ACK, // The message acknowledges the sync
        ACTION, // The message contains a serialized action
        ACTION_ACK, // The message acknowledges an action
        UNDO, // The message undoes a previously sent action
        UNDO_ACK, // The message acknowledges an undo
        QUIT, // The participant or server has quit
    };

    NetworkMessage();

    boost::asio::mutable_buffer headerBuffer();
    boost::asio::mutable_buffer bodyBuffer();

    bool decodeHeader();
    size_t bodySize() const;
    Type getType() const;

    void encodeHandshake();
    std::optional<ApplicationVersion> decodeHandshake();

    void encodeSync(const std::shared_ptr<TournamentStore> & tournament, const std::list<std::pair<ActionId, std::shared_ptr<Action>>> &actionStack);
    void encodeAction(ActionId id, std::shared_ptr<Action> action);
    void encodeQuit();
    void encodeUndo(ActionId actionId);

private:
    void encodeHeader();

    Type mType;
    std::string mHeader;
    std::string mBody;
};

