#pragma once

#include "core/id.hpp"
#include <boost/asio/buffer.hpp>
#include <optional>
#include <rapidjson/stringbuffer.h>

class WebTournamentStore;
class CategoryStore;
class MatchStore;
class PlayerStore;

class JsonBuffer {
public:
    JsonBuffer(rapidjson::StringBuffer buffer);
    JsonBuffer(const JsonBuffer &other) = delete;

    boost::asio::const_buffer getBuffer() {
        return boost::asio::buffer(stringBuffer.GetString(), stringBuffer.GetSize());
    }

private:
    rapidjson::StringBuffer stringBuffer;
};

class JsonEncoder {
public:
    std::unique_ptr<JsonBuffer> encodeSync(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer);
    std::unique_ptr<JsonBuffer> encodeDetailedCategory(const CategoryStore &category);
    std::unique_ptr<JsonBuffer> encodeDetailedPlayer(const PlayerStore &player);

    std::unique_ptr<JsonBuffer> encodeChanges(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer);

private:
    void encodePlayer(const PlayerStore &player);
    void encodeCategory(const PlayerStore &player);
    void encodeMatch(const PlayerStore &player);
};

