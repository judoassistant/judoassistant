#include <rapidjson/writer.h>
#include <rapidjson/document.h>

#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/player_store.hpp"
#include "web/mappers/websocket_json_mapper.hpp"

std::string documentToString(const rapidjson::Document &document) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

std::string WebsocketJSONMapper::mapClockMessage(const std::chrono::milliseconds clock) {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("type", mapString("clock", allocator), allocator);
    document.AddMember("clock", clock.count(), allocator);
    return documentToString(document);
}


std::string WebsocketJSONMapper::mapChangeMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer, std::optional<unsigned int> subscribedTatami, std::chrono::milliseconds clockDiff) {
    rapidjson::Document document;
    document.SetObject();

    auto &allocator = document.GetAllocator();

    rapidjson::Value commandType("tournamentChanges", allocator);
    document.AddMember("type", commandType, allocator);

    // Tournament meta data
    if (tournament.tournamentChanged())
        document.AddMember("tournament", mapTournamentMeta(tournament, allocator), allocator);

    // Categories
    rapidjson::Value categories(rapidjson::kArrayType);
    for (auto categoryId : tournament.getChangedCategories()) {
        const auto &category = tournament.getCategory(categoryId);
        categories.PushBack(mapCategoryMeta(category, allocator), allocator);
    }

    for (auto categoryId : tournament.getAddedCategories()) {
        const auto &category = tournament.getCategory(categoryId);
        categories.PushBack(mapCategoryMeta(category, allocator), allocator);
    }
    document.AddMember("categories", categories, allocator);

    // Erased categories
    rapidjson::Value erasedCategories(rapidjson::kArrayType);
    for (auto categoryId : tournament.getErasedCategories()) {
        rapidjson::Value val(categoryId.getValue());

        erasedCategories.PushBack(val, allocator);
    }
    document.AddMember("erasedCategories", erasedCategories, allocator);

    // Players
    rapidjson::Value players(rapidjson::kArrayType);
    for (auto playerId : tournament.getChangedPlayers()) {
        const auto &player = tournament.getPlayer(playerId);
        players.PushBack(mapPlayerMeta(player, allocator), allocator);
    }

    for (auto playerId : tournament.getAddedPlayers()) {
        const auto &player = tournament.getPlayer(playerId);
        players.PushBack(mapPlayerMeta(player, allocator), allocator);
    }
    document.AddMember("players", players, allocator);

    // Erased players
    rapidjson::Value erasedPlayers(rapidjson::kArrayType);
    for (auto playerId : tournament.getErasedPlayers()) {
        rapidjson::Value val(playerId.getValue());

        erasedPlayers.PushBack(val, allocator);
    }
    document.AddMember("erasedPlayers", erasedPlayers, allocator);

    // Tatamis
    rapidjson::Value tatamis(rapidjson::kArrayType);
    auto tatamiCount = tournament.getTatamis().tatamiCount();
    for (size_t i = 0; i < tatamiCount; ++i) {
        const auto &model = tournament.getWebTatamiModel(i);

        if (model.matchesChanged())
            tatamis.PushBack(mapTatamiMeta(i, model, allocator), allocator);
    }

    document.AddMember("tatamis", tatamis, allocator);

    // Subscibed category or player
    if (subscribedCategory.has_value()) {
        if (tournament.getErasedCategories().find(*subscribedCategory) != tournament.getErasedCategories().end()) {
            document.AddMember("subscribedCategory", rapidjson::Value(), allocator);
        }
        else {
            bool shouldMap = false;
            shouldMap |= (tournament.getChangedCategories().find(*subscribedCategory) != tournament.getChangedCategories().end());
            shouldMap |= (tournament.getAddedCategories().find(*subscribedCategory) != tournament.getAddedCategories().end());
            shouldMap |= (tournament.getCategoryMatchResets().find(*subscribedCategory) != tournament.getCategoryMatchResets().end());
            shouldMap |= (tournament.getCategoryResultsResets().find(*subscribedCategory) != tournament.getCategoryResultsResets().end());
            if (shouldMap)
                document.AddMember("subscribedCategory", mapCategoryFull(tournament, tournament.getCategory(*subscribedCategory), allocator), allocator);
        }
    }
    else if (subscribedPlayer.has_value()) {
        if (tournament.getErasedPlayers().find(*subscribedPlayer) != tournament.getErasedPlayers().end()) {
            document.AddMember("subscribedPlayer", rapidjson::Value(), allocator);
        }
        else {
            bool shouldMap = false;
            shouldMap |= (tournament.getChangedPlayers().find(*subscribedPlayer) != tournament.getChangedPlayers().end());
            shouldMap |= (tournament.getAddedPlayers().find(*subscribedPlayer) != tournament.getAddedPlayers().end());
            shouldMap |= (tournament.getPlayerMatchResets().find(*subscribedPlayer) != tournament.getPlayerMatchResets().end());
            if (shouldMap)
                document.AddMember("subscribedPlayer", mapPlayerFull(tournament.getPlayer(*subscribedPlayer), allocator), allocator);
        }
    }
    else if (subscribedTatami.has_value()) {
        const auto &tatamis = tournament.getTatamis();
        if  (*subscribedTatami < tatamis.tatamiCount()) {
            if (tournament.getWebTatamiModel(*subscribedTatami).changed())
                document.AddMember("subscribedTatami", mapTatamiFull(*subscribedTatami, tournament, allocator), allocator);
        }
    }

    // Subscribed matches
    std::unordered_set<CombinedId> matchIds;
    if (subscribedCategory.has_value() && tournament.containsCategory(*subscribedCategory)) {
        bool matchesReset = (tournament.getCategoryMatchResets().find(*subscribedCategory) != tournament.getCategoryMatchResets().end());
        for (const auto &match : tournament.getCategory(*subscribedCategory).getMatches()) {
            const auto &combinedId = match->getCombinedId();
            if (!matchesReset && tournament.getChangedMatches().find(combinedId) == tournament.getChangedMatches().end())
                continue;
            matchIds.insert(combinedId);
        }
    }
    if (subscribedPlayer.has_value() && tournament.containsPlayer(*subscribedPlayer)) {
        const auto &player = tournament.getPlayer(*subscribedPlayer);
        bool matchesReset = (tournament.getPlayerMatchResets().find(*subscribedPlayer) != tournament.getPlayerMatchResets().end());

        for (const auto &combinedId : player.getMatches()) {
            if (!matchesReset && tournament.getChangedMatches().find(combinedId) == tournament.getChangedMatches().end())
                continue;
            matchIds.insert(combinedId);
        }
    }
    for (size_t i = 0; i < tatamiCount; ++i) {
        const auto &model = tournament.getWebTatamiModel(i);

        // Add all inserted matches
        for (const auto &combinedId : model.getInsertedMatches())
            matchIds.insert(combinedId);

        // Add changed matches that were already present
        for (const auto &combinedId: model.getMatches()) {
            if (tournament.getChangedMatches().find(combinedId) != tournament.getChangedMatches().end())
                matchIds.insert(combinedId);
        }
    }

    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &combinedId : matchIds) {
        const auto &category = tournament.getCategory(combinedId.getCategoryId());
        const auto &match = category.getMatch(combinedId.getMatchId());
        matches.PushBack(mapMatch(category, match, clockDiff, allocator), allocator);
    }
    document.AddMember("matches", matches, allocator);

    return documentToString(document);
}

std::string WebsocketJSONMapper::mapSyncMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer, std::optional<unsigned int> subscribedTatami, std::chrono::milliseconds clockDiff) {
    rapidjson::Document document;
    document.SetObject();

    auto &allocator = document.GetAllocator();

    rapidjson::Value commandType("tournamentSubscription", allocator);
    document.AddMember("type", commandType, allocator);

    // Tournament meta data
    document.AddMember("tournament", mapTournamentMeta(tournament, allocator), allocator);

    // Categories
    rapidjson::Value categories(rapidjson::kArrayType);
    for (const auto &p : tournament.getCategories())
        categories.PushBack(mapCategoryMeta(*(p.second), allocator), allocator);
    document.AddMember("categories", categories, allocator);

    // Players
    rapidjson::Value players(rapidjson::kArrayType);
    for (const auto &p : tournament.getPlayers())
        players.PushBack(mapPlayerMeta(*(p.second), allocator), allocator);
    document.AddMember("players", players, allocator);

    // Tatamis
    rapidjson::Value tatamis(rapidjson::kArrayType);
    auto tatamiCount = tournament.getTatamis().tatamiCount();
    for (size_t i = 0; i < tatamiCount; ++i) {
        const auto &model = tournament.getWebTatamiModel(i);
        tatamis.PushBack(mapTatamiMeta(i, model, allocator), allocator);
    }

    document.AddMember("tatamis", tatamis, allocator);


    // Subscribed player
    if (subscribedPlayer.has_value() && tournament.containsPlayer(*subscribedPlayer))
        document.AddMember("subscribedPlayer", mapPlayerFull(tournament.getPlayer(*subscribedPlayer), allocator), allocator);
    else
        document.AddMember("subscribedPlayer", rapidjson::Value(), allocator);

    // Subscribed category
    if (subscribedCategory.has_value() && tournament.containsCategory(*subscribedCategory))
        document.AddMember("subscribedCategory", mapCategoryFull(tournament, tournament.getCategory(*subscribedCategory), allocator), allocator);
    else
        document.AddMember("subscribedCategory", rapidjson::Value(), allocator);

    // Subscribed tatami
    if (subscribedTatami.has_value() && *subscribedTatami < tournament.getTatamis().tatamiCount())
        document.AddMember("subscribedTatami", mapTatamiFull(*subscribedTatami, tournament, allocator), allocator);
    else
        document.AddMember("subscribedTatami", rapidjson::Value(), allocator);


    // Subscribed matches
    std::unordered_set<CombinedId> matchIds;
    if (subscribedCategory.has_value() && tournament.containsCategory(*subscribedCategory)) {
        for (const auto &match : tournament.getCategory(*subscribedCategory).getMatches())
            matchIds.insert(match->getCombinedId());
    }
    if (subscribedPlayer.has_value() && tournament.containsPlayer(*subscribedPlayer)) {
        const auto &player = tournament.getPlayer(*subscribedPlayer);

        for (const auto &combinedId : player.getMatches())
            matchIds.insert(combinedId);
    }
    for (size_t i = 0; i < tatamiCount; ++i) {
        const auto &model = tournament.getWebTatamiModel(i);
        for (const auto &combinedId: model.getMatches()) {
            matchIds.insert(combinedId);
        }
    }

    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &combinedId : matchIds) {
        const auto &category = tournament.getCategory(combinedId.getCategoryId());
        const auto &match = category.getMatch(combinedId.getMatchId());
        matches.PushBack(mapMatch(category, match, clockDiff, allocator), allocator);
    }

    document.AddMember("matches", matches, allocator);

    return documentToString(document);
}

std::string WebsocketJSONMapper::mapSubscribeTournamentFailMessage() {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("type", mapString("tournamentSubscriptionFail", allocator), allocator);
    return documentToString(document);
}

std::string WebsocketJSONMapper::mapListTournamentsMessage(const std::vector<TournamentMeta> &pastTournaments, const std::vector<TournamentMeta> &upcomingTournaments) {
    // TODO: Implement
    return "";
}

std::string WebsocketJSONMapper::mapListTournamentsFailMessage() {
    // TODO: Implement
    return "";
}

std::string WebsocketJSONMapper::mapSubscribeCategoryMessage(const WebTournamentStore &tournament, const CategoryStore &category, std::chrono::milliseconds clockDiff) {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("type", mapString("categorySubscription", allocator), allocator);
    document.AddMember("subscribedCategory", mapCategoryFull(tournament, category, allocator), allocator);

    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &match : category.getMatches())
        matches.PushBack(mapMatch(category, *match, clockDiff, allocator), allocator);
    document.AddMember("matches", matches, allocator);

    return documentToString(document);
}

std::string WebsocketJSONMapper::mapSubscribeCategoryFailMessage() {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("type", mapString("categorySubscriptionFail", allocator), allocator);
    return documentToString(document);
}

std::string WebsocketJSONMapper::mapSubscribePlayerMessage(const WebTournamentStore &tournament, const PlayerStore &player, std::chrono::milliseconds clockDiff) {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("type", mapString("playerSubscription", allocator), allocator);
    document.AddMember("subscribedPlayer", mapPlayerFull(player, allocator), allocator);

    rapidjson::Value matches(rapidjson::kArrayType);
    for (auto combinedId : player.getMatches()) {
        const auto &category = tournament.getCategory(combinedId.getCategoryId());
        const auto &match = category.getMatch(combinedId.getMatchId());
        matches.PushBack(mapMatch(category, match, clockDiff, allocator), allocator);
    }
    document.AddMember("matches", matches, allocator);

    return documentToString(document);
}

std::string WebsocketJSONMapper::mapSubscribePlayerFailMessage() {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("type", mapString("playerSubscriptionFail", allocator), allocator);
    return documentToString(document);
}

std::string WebsocketJSONMapper::mapSubscribeTatamiMessage(const WebTournamentStore &tournament, size_t index, std::chrono::milliseconds clockDiff) {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("type", mapString("tatamiSubscription", allocator), allocator);
    document.AddMember("subscribedTatami", mapTatamiFull(index, tournament, allocator), allocator);

    return documentToString(document);
}

std::string WebsocketJSONMapper::mapSubscribeTatamiFailMessage() {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    document.SetObject();
    document.AddMember("type", mapString("tatamiSubscriptionFail", allocator), allocator);
    return documentToString(document);
}

rapidjson::Value WebsocketJSONMapper::mapTournamentMeta(const WebTournamentStore &tournament, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();
    res.AddMember("name", mapString(tournament.getName(), allocator), allocator);
    res.AddMember("location", mapString(tournament.getLocation(), allocator), allocator);
    res.AddMember("date", mapString(tournament.getDate(), allocator), allocator);
    res.AddMember("webName", mapString(tournament.getWebName(), allocator), allocator);
    res.AddMember("tatamiCount", tournament.getTatamis().tatamiCount(), allocator);
    return res;
}

rapidjson::Value WebsocketJSONMapper::mapPlayerMeta(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();
    res.AddMember("id", player.getId().getValue(), allocator);
    res.AddMember("firstName", mapString(player.getFirstName(), allocator), allocator);
    res.AddMember("lastName", mapString(player.getLastName(), allocator), allocator);

    if (player.getCountry().has_value())
        res.AddMember("country", mapString(player.getCountry()->toString(), allocator), allocator);
    else
        res.AddMember("country", rapidjson::Value(), allocator);

    if (player.getRank().has_value())
        res.AddMember("rank", mapString(player.getRank()->toString(), allocator), allocator);
    else
        res.AddMember("rank", rapidjson::Value(), allocator);

    res.AddMember("club", mapString(player.getClub(), allocator), allocator);

    return res;
}

rapidjson::Value WebsocketJSONMapper::mapPlayerFull(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res = mapPlayerMeta(player, allocator);

    rapidjson::Value categories(rapidjson::kArrayType);
    for (const auto &categoryId : player.getCategories()) {
        categories.PushBack(categoryId.getValue(), allocator);
    }
    res.AddMember("categories", categories, allocator);

    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &combinedId : player.getMatches()) {
        matches.PushBack(mapCombinedId(combinedId, allocator), allocator);
    }
    res.AddMember("matches", matches, allocator);

    return res;
}

rapidjson::Value WebsocketJSONMapper::mapCategoryMeta(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();
    res.AddMember("id", category.getId().getValue(), allocator);
    res.AddMember("name", mapString(category.getName(), allocator), allocator);
    res.AddMember("matchesHidden", category.areMatchesHidden(), allocator);
    return res;
}

rapidjson::Value WebsocketJSONMapper::mapCategoryFull(const TournamentStore &tournament, const CategoryStore &category, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res = mapCategoryMeta(category, allocator);

    // Matches
    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &match : category.getMatches())
        matches.PushBack(mapCombinedId(match->getCombinedId(), allocator), allocator);

    res.AddMember("matches", matches, allocator);

    // Players
    rapidjson::Value players(rapidjson::kArrayType);
    for (auto &playerId : category.getPlayers()) {
        rapidjson::Value val(playerId.getValue());
        players.PushBack(val, allocator);
    }
    res.AddMember("players", players, allocator);

    // Category Results
    rapidjson::Value categoryResults(rapidjson::kArrayType);
    for (const auto &row : category.getDrawSystem().getResults(tournament, category)) {
        rapidjson::Value val;
        val.SetObject();

        val.AddMember("player", row.first.getValue(), allocator);

        rapidjson::Value pos;
        if (row.second.has_value())
            pos.Set(*(row.second), allocator);
        val.AddMember("pos", pos, allocator);

        res.PushBack(val, allocator);
    }

    res.AddMember("results", categoryResults, allocator);

    return res;
}

rapidjson::Value WebsocketJSONMapper::mapTatamiMeta(size_t index, const WebTatamiModel &model, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();
    res.AddMember("index", index, allocator);

    rapidjson::Value matches(rapidjson::kArrayType);
    for (auto &combinedId : model.getMatches()) {
        matches.PushBack(mapCombinedId(combinedId, allocator), allocator);
    }
    res.AddMember("matches", matches, allocator);

    return res;
}

rapidjson::Value WebsocketJSONMapper::mapTatamiFull(size_t index, const WebTournamentStore &tournament, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();
    res.AddMember("index", index, allocator);

    const auto &tatami = tournament.getTatamis().at(index);
    rapidjson::Value concurrentGroups(rapidjson::kArrayType);
    for (size_t i = 0; i < tatami.groupCount(); ++i) {
        const auto &concurrentGroup = tatami.at(i);

        rapidjson::Value sequentialGroups(rapidjson::kArrayType);
        for (size_t j = 0; j < concurrentGroup.groupCount(); ++j) {
            const auto &sequentialGroup = concurrentGroup.at(j);

            rapidjson::Value blocks(rapidjson::kArrayType);
            for (size_t k = 0; k < sequentialGroup.blockCount(); ++k) {
                auto p = sequentialGroup.at(k);

                CategoryId categoryId = p.first;
                MatchType matchType = p.second;
                rapidjson::Value block;
                block.SetObject();
                block.AddMember("categoryId", categoryId.getValue(), allocator);
                block.AddMember("type", mapMatchType(matchType, allocator), allocator);
                block.AddMember("status", mapBlockStatus(tournament.getCategory(categoryId), matchType, allocator), allocator);

                blocks.PushBack(block, allocator);
            }
            sequentialGroups.PushBack(blocks, allocator);
        }
        concurrentGroups.PushBack(sequentialGroups, allocator);
    }
    res.AddMember("blocks", concurrentGroups, allocator);

    return res;
}

rapidjson::Value WebsocketJSONMapper::mapMatch(const CategoryStore &category, const MatchStore &match, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();
    res.AddMember("combinedId", mapCombinedId(match.getCombinedId(), allocator), allocator);
    res.AddMember("bye", match.isBye(), allocator);
    res.AddMember("title", mapString(match.getTitle(), allocator), allocator);
    res.AddMember("position", category.getMatchPosition(match.getId()), allocator);

    if (match.getWhitePlayer().has_value())
        res.AddMember("whitePlayer", match.getWhitePlayer()->getValue(), allocator);
    else
        res.AddMember("whitePlayer", rapidjson::Value(), allocator);

    if (match.getBluePlayer().has_value())
        res.AddMember("bluePlayer", match.getBluePlayer()->getValue(), allocator);
    else
        res.AddMember("bluePlayer", rapidjson::Value(), allocator);

    res.AddMember("status", mapMatchStatus(match.getStatus(), allocator), allocator);
    res.AddMember("whiteScore", mapMatchScore(match.getWhiteScore(), allocator), allocator);
    res.AddMember("blueScore", mapMatchScore(match.getBlueScore(), allocator), allocator);
    res.AddMember("goldenScore", match.isGoldenScore(), allocator);

    if (match.getStatus() == MatchStatus::UNPAUSED)
        res.AddMember("resumeTime", mapTime(match.getResumeTime(), clockDiff, allocator), allocator);
    else
        res.AddMember("resumeTime", rapidjson::Value(), allocator);

    res.AddMember("duration", mapDuration(match.getDuration(), allocator), allocator);
    res.AddMember("normalTime", mapDuration(category.getRuleset().getNormalTime(), allocator), allocator);
    res.AddMember("osaekomi", mapOsaekomi(match.getOsaekomi(), clockDiff, allocator), allocator);

    std::optional<MatchStore::PlayerIndex> winner;
    if (match.getStatus() == MatchStatus::FINISHED)
        winner = category.getRuleset().getWinner(match);
    if (winner.has_value())
        res.AddMember("winner", mapPlayerIndex(*winner, allocator), allocator);
    else
        res.AddMember("winner", rapidjson::Value(), allocator);

    rapidjson::Value events(rapidjson::kArrayType);
    for (const MatchEvent &event : match.getEvents())
        events.PushBack(mapMatchEvent(event, allocator), allocator);

    res.AddMember("events", events, allocator);

    return res;
}

rapidjson::Value WebsocketJSONMapper::mapBlockStatus(const CategoryStore &category, MatchType matchType, rapidjson::Document::AllocatorType &allocator) {
    const auto &status = category.getStatus(matchType);
    std::string statusStr = "FINISHED";
    if (status.notStartedMatches > 0 && status.startedMatches == 0 && status.finishedMatches == 0) {
        statusStr ="NOT_STARTED";
    } else if (status.startedMatches > 0 || status.notStartedMatches > 0) {
        statusStr = "STARTED";
    }
    return mapString(statusStr, allocator);
}

rapidjson::Value WebsocketJSONMapper::mapCombinedId(const CombinedId &id, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();
    res.AddMember("categoryId", id.getCategoryId().getValue(), allocator);
    res.AddMember("matchId", id.getMatchId().getValue(), allocator);
    return res;
}

rapidjson::Value WebsocketJSONMapper::mapDuration(const std::chrono::milliseconds &duration, rapidjson::Document::AllocatorType &allocator) {
    return rapidjson::Value(duration.count());
}

rapidjson::Value WebsocketJSONMapper::mapMatchEvent(const MatchEvent &event, rapidjson::Document::AllocatorType &allocator) {
    std::string typeStr;
    if (event.type == MatchEventType::IPPON) {
        typeStr = "IPPON";
    } else if (event.type == MatchEventType::WAZARI) {
        typeStr = "WAZARI";
    } else if (event.type == MatchEventType::SHIDO) {
        typeStr = "SHIDO";
    } else if (event.type == MatchEventType::HANSOKU_MAKE) {
        typeStr = "HANSOKU_MAKE";
    } else if (event.type == MatchEventType::IPPON_OSAEKOMI) {
        typeStr = "IPPON_OSAEKOMI";
    } else if (event.type == MatchEventType::WAZARI_OSAEKOMI) {
        typeStr = "WAZARI_OSAEKOMI";
    } else if (event.type == MatchEventType::CANCEL_IPPON) {
        typeStr = "CANCEL_IPPON";
    } else if (event.type == MatchEventType::CANCEL_WAZARI) {
        typeStr = "CANCEL_WAZARI";
    } else if (event.type == MatchEventType::CANCEL_SHIDO) {
        typeStr = "CANCEL_SHIDO";
    } else {
        typeStr = "CANCEL_HANSOKU_MAKE";
    }

    rapidjson::Value res;
    res.SetObject();
    res.AddMember("type", mapString(typeStr, allocator), allocator);
    res.AddMember("playerIndex", mapPlayerIndex(event.playerIndex, allocator), allocator);
    res.AddMember("duration", mapDuration(event.duration, allocator), allocator);
    return res;
}

rapidjson::Value WebsocketJSONMapper::mapMatchScore(const MatchStore::Score &score, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();
    res.AddMember("ippon", score.ippon, allocator);
    res.AddMember("wazari", score.wazari, allocator);
    res.AddMember("shido", score.shido, allocator);
    res.AddMember("hansokuMake", score.hansokuMake, allocator);
    return res;
}

rapidjson::Value WebsocketJSONMapper::mapMatchStatus(const MatchStatus &status, rapidjson::Document::AllocatorType &allocator) {
    std::string statusStr = "FINISHED";
    if (status == MatchStatus::NOT_STARTED) {
        statusStr = "NOT_STARTED";
    } else if (status == MatchStatus::PAUSED) {
        statusStr = "PAUSED";
    } else if (status == MatchStatus::UNPAUSED) {
        statusStr = "UNPAUSED";
    }
    return mapString(statusStr, allocator);
}

rapidjson::Value WebsocketJSONMapper::mapMatchType(MatchType matchType, rapidjson::Document::AllocatorType &allocator) {
    std::string typeStr = "FINAL";
    if (matchType == MatchType::ELIMINATION) {
        typeStr = "ELIMINATION";
    }
    return mapString(typeStr, allocator);
}

rapidjson::Value WebsocketJSONMapper::mapOsaekomi(const std::optional<std::pair<MatchStore::PlayerIndex, std::chrono::milliseconds>>& osaekomi, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    if (!osaekomi.has_value()) {
        return res;
    }

    res.SetObject();
    res.AddMember("player", mapPlayerIndex(osaekomi->first, allocator), allocator);
    res.AddMember("start", mapTime(osaekomi->second, clockDiff, allocator), allocator);
    return res;
}

rapidjson::Value WebsocketJSONMapper::mapPlayerIndex(const MatchStore::PlayerIndex playerIndex, rapidjson::Document::AllocatorType &allocator) {
    std::string playerIndexStr = "WHITE";
    if (playerIndex == MatchStore::PlayerIndex::BLUE) {
        playerIndexStr = "BLUE";
    }
    return mapString(playerIndexStr, allocator);
}

rapidjson::Value WebsocketJSONMapper::mapString(const std::string &str, rapidjson::Document::AllocatorType &allocator) {
    return rapidjson::Value(str.c_str(), str.size(), allocator);
}

rapidjson::Value WebsocketJSONMapper::mapTime(const std::chrono::milliseconds &time, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator) {
    auto sinceEpoch = time - clockDiff;
    return rapidjson::Value(sinceEpoch.count()); // unix timestamp
}
