#pragma once

#include <boost/asio/buffer.hpp>
#include <optional>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

#include "core/id.hpp"

class WebTournamentStore;
class CategoryStore;
class MatchStore;
class PlayerStore;

class JsonBuffer {
public:
    JsonBuffer();
    JsonBuffer(const JsonBuffer &other) = delete;
    boost::asio::const_buffer getBuffer() const;
    rapidjson::StringBuffer& getStringBuffer();

private:
    rapidjson::StringBuffer mStringBuffer;
};

class JsonEncoder {
public:
    std::unique_ptr<JsonBuffer> encodeSyncMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer);
    std::unique_ptr<JsonBuffer> encodeDetailedCategoryMessage(const CategoryStore &category);
    std::unique_ptr<JsonBuffer> encodeDetailedPlayerMessage(const PlayerStore &player);

    std::unique_ptr<JsonBuffer> encodeChangesMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer);

private:
    rapidjson::Value encodeMeta(const WebTournamentStore &tournament, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value encodePlayer(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeDetailedPlayer(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value encodeCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeDetailedCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value encodeMatch(const MatchStore &match, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeString(const std::string &str, rapidjson::Document::AllocatorType &allocator);

};

