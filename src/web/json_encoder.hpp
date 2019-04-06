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
    std::unique_ptr<JsonBuffer> encodeTournamentSubscriptionMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer);
    std::unique_ptr<JsonBuffer> encodeTournamentSubscriptionFailMessage();
    std::unique_ptr<JsonBuffer> encodeTournamentChangesMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer);

    std::unique_ptr<JsonBuffer> encodeCategorySubscriptionMessage(const WebTournamentStore &tournament, const CategoryStore &category);
    std::unique_ptr<JsonBuffer> encodeCategorySubscriptionFailMessage();

    std::unique_ptr<JsonBuffer> encodePlayerSubscriptionMessage(const WebTournamentStore &tournament, const PlayerStore &player);
    std::unique_ptr<JsonBuffer> encodePlayerSubscriptionFailMessage();

private:
    rapidjson::Value encodeMeta(const WebTournamentStore &tournament, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value encodePlayer(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeSubscribedPlayer(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value encodeCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeSubscribedCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value encodeMatch(const MatchStore &match, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeString(const std::string &str, rapidjson::Document::AllocatorType &allocator);

};

