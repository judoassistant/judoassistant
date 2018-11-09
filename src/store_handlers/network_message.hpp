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

    NetworkMessage() {
        mHeader.resize(HEADER_LENGTH);
    }

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

    boost::asio::mutable_buffer headerBuffer() {
        return boost::asio::buffer(mHeader, HEADER_LENGTH);
    }

    boost::asio::mutable_buffer bodyBuffer() {
        return boost::asio::buffer(mBody.data(), mBody.size());
    }

    size_t bodySize() const {
        return mBody.size();
    }

    bool decodeHeader() {
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

    Type getType() const {
        return mType;
    }

    void encodeHandshake() {
        mType = Type::HANDSHAKE;

        std::ostringstream stream;
        cereal::PortableBinaryOutputArchive archive(stream);
        archive(ApplicationVersion::current());

        mBody = stream.str();
        encodeHeader();
    }

    std::optional<ApplicationVersion> decodeHandshake() {
        std::istringstream stream(mBody);
        cereal::PortableBinaryInputArchive archive(stream);

        try {
            ApplicationVersion version;
            archive(version);
            return version;
        }
        catch (const std::exception &e) {
            return std::nullopt;
        }
    }

    void encodeSync(const std::shared_ptr<TournamentStore> & tournament, const std::list<std::pair<ActionId, std::shared_ptr<Action>>> &actionStack) {
        mType = Type::SYNC;

        std::ostringstream stream;
        cereal::PortableBinaryOutputArchive archive(stream);
        archive(tournament, actionStack);

        mBody = stream.str();
        encodeHeader();
    }

    void encodeAction(ActionId id, std::shared_ptr<Action> action) {
        // TODO: implement
        mType = Type::ACTION;

        std::ostringstream stream;
        cereal::PortableBinaryOutputArchive archive(stream);
        archive(id, action);

        mBody = stream.str();
        encodeHeader();
    }

    void encodeQuit() {
        mType = Type::QUIT;
        mBody.clear();
        encodeHeader();
    }

private:
    void encodeHeader() {
        std::ostringstream stream;
        cereal::PortableBinaryOutputArchive archive(stream);

        size_t bodyLength = mBody.size();
        archive(bodyLength, mType);

        mHeader = stream.str();
        assert(mHeader.size() == HEADER_LENGTH);
    }

    Type mType;
    std::string mHeader;
    std::string mBody;
};

