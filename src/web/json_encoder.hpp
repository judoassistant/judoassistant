#pragma once

#include "core/id.hpp"
#include <boost/asio/buffer.hpp>
#include <optional>
#include <rapidjson/stringbuffer.h>

class WebTournamentStore;
class CategoryStore;
class MatchStore;
class PlayerStore;

struct JsonBuffer {
    rapidjson::StringBuffer stringBuffer;

    boost::asio::const_buffer getBuffer() {
        return boost::asio::buffer(stringBuffer.GetString(), stringBuffer.GetSize());
    }
};

class JsonEncoder {
public:
    std::unique_ptr<JsonBuffer> encodeSync(const WebTournamentStore &tournament, std::optional<MatchId> subscribedCategory, std::optional<PlayerId> subscribedPlayer);
    std::unique_ptr<JsonBuffer> encodeDetailedCategory(const CategoryStore &category);
    std::unique_ptr<JsonBuffer> encodeDetailedPlayer(const PlayerStore &player);

    std::unique_ptr<JsonBuffer> encodeChanges(const WebTournamentStore &tournament, std::optional<MatchId> subscribedCategory, std::optional<PlayerId> subscribedPlayer);

private:
    void encodePlayer(const PlayerStore &player);
    void encodeCategory(const PlayerStore &player);
    void encodeMatch(const PlayerStore &player);
};
