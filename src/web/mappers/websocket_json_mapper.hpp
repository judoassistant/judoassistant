#pragma once

#include <chrono>
#include <string>
#include <rapidjson/document.h>

#include "web/web_tournament_store.hpp"

#include "core/stores/match_store.hpp"

class WebsocketJSONMapper {
public:
    std::string mapClockMessage(std::chrono::milliseconds clock);
    std::string mapChangeMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer, std::optional<unsigned int> subscribedTatami, std::chrono::milliseconds clockDiff);

    std::string mapSyncMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer, std::optional<unsigned int> subscribedTatami, std::chrono::milliseconds clockDiff);

private:
    rapidjson::Value mapTournamentMeta(const WebTournamentStore &tournament, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value mapCategoryFull(const TournamentStore &tournament, const CategoryStore &category, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapCategoryMeta(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value mapMatch(const CategoryStore &category, const MatchStore &match, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value mapPlayerFull(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapPlayerMeta(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator);

    rapidjson::Value mapTatamiFull(size_t index, const WebTournamentStore &tournament, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapTatamiMeta(size_t index, const WebTatamiModel &model, rapidjson::Document::AllocatorType &allocator);

    // Mapper helpers for primitives
    rapidjson::Value mapBlockStatus(const CategoryStore &category, MatchType matchType, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapCombinedId(const CombinedId &id, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapDuration(const std::chrono::milliseconds &duration, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapMatchEvent(const MatchEvent &event, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapMatchScore(const MatchStore::Score &score, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapMatchStatus(const MatchStatus &status, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapMatchType(MatchType matchType, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapOsaekomi(const std::optional<std::pair<MatchStore::PlayerIndex, std::chrono::milliseconds>>& osaekomi, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapPlayerIndex(const MatchStore::PlayerIndex playerIndex, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapString(const std::string &str, rapidjson::Document::AllocatorType &allocator);
    rapidjson::Value mapTime(const std::chrono::milliseconds &time, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator);
};
