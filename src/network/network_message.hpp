#pragma once

#include <boost/asio/buffer.hpp>
#include <list>
#include <sstream>
#include <variant>

#include "core.hpp"
#include "id.hpp"

class ApplicationVersion;
class TournamentStore;
class Action;

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
    bool decodeHandshake(ApplicationVersion &version);

    typedef std::list<std::pair<ActionId, std::shared_ptr<Action>>> ActionList;
    void encodeSync(const TournamentStore & tournament, const ActionList &actionStack);
    bool decodeSync(TournamentStore & tournament, ActionList &actionStack);

    void encodeSyncAck();

    void encodeAction(const ClientId &clientId, const ActionId &actionId, const std::shared_ptr<Action> &action);
    bool decodeAction(ClientId &clientId, ActionId &actionId, std::shared_ptr<Action> &action);

    void encodeActionAck(const ActionId &actionId);
    bool decodeActionAck(ActionId &actionId);

    void encodeQuit();

    void encodeUndo(const ActionId &actionId);
    bool decodeUndo(ActionId &actionId);

    void encodeUndoAck(const ActionId &actionId);
    bool decodeUndoAck(ActionId &actionId);

private:
    void encodeHeader();

    Type mType;
    std::string mHeader;
    std::string mBody;
};

