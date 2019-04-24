#pragma once

#include <boost/asio/buffer.hpp>
#include <optional>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

#include "core/id.hpp"
#include "core/stores/match_store.hpp"

class WebTournamentStore;
class CategoryStore;
class PlayerStore;
class WebTatamiModel;

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
    std::unique_ptr<JsonBuffer> encodeTournamentSubscriptionMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer, std::chrono::milliseconds clockDiff);
    std::unique_ptr<JsonBuffer> encodeTournamentSubscriptionFailMessage();
    std::unique_ptr<JsonBuffer> encodeTournamentChangesMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer, std::chrono::milliseconds clockDiff);
    bool hasTournamentChanges(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer);

    std::unique_ptr<JsonBuffer> encodeCategorySubscriptionMessage(const WebTournamentStore &tournament, const CategoryStore &category, std::chrono::milliseconds clockDiff);
    std::unique_ptr<JsonBuffer> encodeCategorySubscriptionFailMessage();

    std::unique_ptr<JsonBuffer> encodePlayerSubscriptionMessage(const WebTournamentStore &tournament, const PlayerStore &player, std::chrono::milliseconds clockDiff);
    std::unique_ptr<JsonBuffer> encodePlayerSubscriptionFailMessage();

private:
    rapidjson::Value encodeMeta(const WebTournamentStore &tournament, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value encodePlayer(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeSubscribedPlayer(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value encodeCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeSubscribedCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value encodeTatami(size_t index, const WebTatamiModel &model, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value encodeMatch(const CategoryStore &category, const MatchStore &match, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeString(const std::string &str, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeCombinedId(const std::pair<CategoryId, MatchId> &id, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeMatchScore(const MatchStore::Score &score, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeMatchStatus(const MatchStatus &status, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeMatchEvent(const MatchEvent &event, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeDuration(const std::chrono::milliseconds &duration, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeTime(const std::chrono::milliseconds &time, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value encodeOsaekomi(const std::optional<std::pair<MatchStore::PlayerIndex, std::chrono::milliseconds>>& osaekomi, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator);
};

