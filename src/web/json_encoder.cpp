#include <rapidjson/writer.h>

#include "core/draw_systems/draw_system.hpp"
#include "core/log.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "web/database.hpp"
#include "web/json_encoder.hpp"
#include "web/web_tatami_model.hpp"
#include "web/web_tournament_store.hpp"

JsonBuffer::JsonBuffer() {

}

boost::asio::const_buffer JsonBuffer::getBuffer() const {
    return boost::asio::buffer(mStringBuffer.GetString(), mStringBuffer.GetSize());
}

rapidjson::StringBuffer& JsonBuffer::getStringBuffer() {
    return mStringBuffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeTournamentSubscriptionMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer, std::optional<unsigned int> subscribedTatami, std::chrono::milliseconds clockDiff, bool shouldCache) {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    // See if values common to all participants are already cached
    if (mCachedSubscriptionObject.has_value()) {
        document.CopyFrom(*mCachedSubscriptionObject, allocator);
    }
    else {
        document.SetObject();

        document.AddMember("type", encodeString("tournamentSubscription", allocator), allocator);

        // Tournament meta data field
        document.AddMember("tournament", encodeMeta(tournament, allocator), allocator);

        // categories field
        rapidjson::Value categories(rapidjson::kArrayType);
        for (const auto &p : tournament.getCategories())
            categories.PushBack(encodeCategory(*(p.second), allocator), allocator);
        document.AddMember("categories", categories, allocator);

        // players field
        rapidjson::Value players(rapidjson::kArrayType);
        for (const auto &p : tournament.getPlayers())
            players.PushBack(encodePlayer(*(p.second), allocator), allocator);
        document.AddMember("players", players, allocator);

        // Encode tatamis
        rapidjson::Value tatamis(rapidjson::kArrayType);
        auto tatamiCount = tournament.getTatamis().tatamiCount();
        for (size_t i = 0; i < tatamiCount; ++i) {
            const auto &model = tournament.getWebTatamiModel(i);
            tatamis.PushBack(encodeTatami(i, model, allocator), allocator);
        }

        document.AddMember("tatamis", tatamis, allocator);

        // Save to cache
        if (shouldCache) {
            mCachedSubscriptionObject = rapidjson::Document();
            mCachedSubscriptionObject->CopyFrom(document, mCachedSubscriptionObject->GetAllocator());
        }
    }

    // subscribed category field
    if (subscribedPlayer.has_value() && tournament.containsPlayer(*subscribedPlayer))
        document.AddMember("subscribedPlayer", encodeSubscribedPlayer(tournament.getPlayer(*subscribedPlayer), allocator), allocator);
    else
        document.AddMember("subscribedPlayer", rapidjson::Value(), allocator);

    // subscribed category field
    if (subscribedCategory.has_value() && tournament.containsCategory(*subscribedCategory))
        document.AddMember("subscribedCategory", encodeSubscribedCategory(tournament, tournament.getCategory(*subscribedCategory), allocator), allocator);
    else
        document.AddMember("subscribedCategory", rapidjson::Value(), allocator);

    // subscribed tatami field
    const auto &tatamis = tournament.getTatamis();
    if (subscribedTatami.has_value() && *subscribedTatami < tatamis.tatamiCount())
        document.AddMember("subscribedTatami", encodeSubscribedTatami(*subscribedTatami, tournament, allocator), allocator);
    else
        document.AddMember("subscribedTatami", rapidjson::Value(), allocator);


    // Identify matches
    std::unordered_set<CombinedId> matchIds;

    // Identity subscribed matches
    if (subscribedCategory.has_value() && tournament.containsCategory(*subscribedCategory)) {
        for (const auto &match : tournament.getCategory(*subscribedCategory).getMatches())
            matchIds.insert(match->getCombinedId());
    }
    else if (subscribedPlayer.has_value() && tournament.containsPlayer(*subscribedPlayer)) {
        const auto &player = tournament.getPlayer(*subscribedPlayer);

        for (const auto &combinedId : player.getMatches())
            matchIds.insert(combinedId);
    }

    // Identify tatami matches
    auto tatamiCount = tournament.getTatamis().tatamiCount();
    for (size_t i = 0; i < tatamiCount; ++i) {
        const auto &model = tournament.getWebTatamiModel(i);
        for (const auto &combinedId: model.getMatches()) {
            matchIds.insert(combinedId);
        }
    }

    // Encode matches
    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &combinedId : matchIds) {
        const auto &category = tournament.getCategory(combinedId.getCategoryId());
        const auto &match = category.getMatch(combinedId.getMatchId());
        matches.PushBack(encodeMatch(category, match, clockDiff, allocator, shouldCache), allocator);
    }

    document.AddMember("matches", matches, allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeCategorySubscriptionMessage(const WebTournamentStore &tournament, const CategoryStore &category, std::chrono::milliseconds clockDiff) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("type", encodeString("categorySubscription", allocator), allocator);

    // subscribed category field
    document.AddMember("subscribedCategory", encodeSubscribedCategory(tournament, category, allocator), allocator);

    // matches
    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &match : category.getMatches())
        matches.PushBack(encodeMatch(category, *match, clockDiff, allocator, false), allocator);
    document.AddMember("matches", matches, allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodePlayerSubscriptionMessage(const WebTournamentStore &tournament, const PlayerStore &player, std::chrono::milliseconds clockDiff) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("type", encodeString("playerSubscription", allocator), allocator);

    // subscribed category field
    document.AddMember("subscribedPlayer", encodeSubscribedPlayer(player, allocator), allocator);

    // matches
    rapidjson::Value matches(rapidjson::kArrayType);
    for (auto combinedId : player.getMatches()) {
        const auto &category = tournament.getCategory(combinedId.getCategoryId());
        const auto &match = category.getMatch(combinedId.getMatchId());
        matches.PushBack(encodeMatch(category, match, clockDiff, allocator, false), allocator);
    }
    document.AddMember("matches", matches, allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeTatamiSubscriptionMessage(const WebTournamentStore &tournament, size_t index, std::chrono::milliseconds clockDiff) {
    assert(index < tournament.getTatamis().tatamiCount());

    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("type", encodeString("tatamiSubscription", allocator), allocator);

    // subscribed category field
    document.AddMember("subscribedTatami", encodeSubscribedTatami(index, tournament, allocator), allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeTournamentChangesMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer, std::optional<unsigned int> subscribedTatami, std::chrono::milliseconds clockDiff) {
    rapidjson::Document document;
    auto &allocator = document.GetAllocator();

    if (mCachedChangesObject.has_value()) {
        document.CopyFrom(*mCachedChangesObject, allocator);
    }
    else {
        document.SetObject();
        document.AddMember("type", encodeString("tournamentChanges", allocator), allocator);

        // Tournament meta data field
        if (tournament.tournamentChanged())
            document.AddMember("tournament", encodeMeta(tournament, allocator), allocator);

        // categories field
        rapidjson::Value categories(rapidjson::kArrayType);
        for (auto categoryId : tournament.getChangedCategories()) {
            const auto &category = tournament.getCategory(categoryId);
            categories.PushBack(encodeCategory(category, allocator), allocator);
        }

        for (auto categoryId : tournament.getAddedCategories()) {
            const auto &category = tournament.getCategory(categoryId);
            categories.PushBack(encodeCategory(category, allocator), allocator);
        }
        document.AddMember("categories", categories, allocator);

        // erased categories field
        rapidjson::Value erasedCategories(rapidjson::kArrayType);
        for (auto categoryId : tournament.getErasedCategories()) {
            rapidjson::Value val(categoryId.getValue());

            erasedCategories.PushBack(val, allocator);
        }
        document.AddMember("erasedCategories", erasedCategories, allocator);

        // players field
        rapidjson::Value players(rapidjson::kArrayType);
        for (auto playerId : tournament.getChangedPlayers()) {
            const auto &player = tournament.getPlayer(playerId);
            players.PushBack(encodePlayer(player, allocator), allocator);
        }

        for (auto playerId : tournament.getAddedPlayers()) {
            const auto &player = tournament.getPlayer(playerId);
            players.PushBack(encodePlayer(player, allocator), allocator);
        }
        document.AddMember("players", players, allocator);

        // erased players field
        rapidjson::Value erasedPlayers(rapidjson::kArrayType);
        for (auto playerId : tournament.getErasedPlayers()) {
            rapidjson::Value val(playerId.getValue());

            erasedPlayers.PushBack(val, allocator);
        }
        document.AddMember("erasedPlayers", erasedPlayers, allocator);

        // Encode tatamis
        rapidjson::Value tatamis(rapidjson::kArrayType);
        auto tatamiCount = tournament.getTatamis().tatamiCount();
        for (size_t i = 0; i < tatamiCount; ++i) {
            const auto &model = tournament.getWebTatamiModel(i);

            if (model.matchesChanged())
                tatamis.PushBack(encodeTatami(i, model, allocator), allocator);
        }

        document.AddMember("tatamis", tatamis, allocator);

        // save object to cache
        mCachedChangesObject = rapidjson::Document();
        mCachedChangesObject->CopyFrom(document, mCachedChangesObject->GetAllocator());
    }

    // Encode subscribed category or player
    if (subscribedCategory.has_value()) { // encode subscribed category
        if (tournament.getErasedCategories().find(*subscribedCategory) != tournament.getErasedCategories().end()) {
            document.AddMember("subscribedCategory", rapidjson::Value(), allocator);
        }
        else {
            bool shouldEncode = false;

            shouldEncode |= (tournament.getChangedCategories().find(*subscribedCategory) != tournament.getChangedCategories().end());
            shouldEncode |= (tournament.getAddedCategories().find(*subscribedCategory) != tournament.getAddedCategories().end());
            shouldEncode |= (tournament.getCategoryMatchResets().find(*subscribedCategory) != tournament.getCategoryMatchResets().end());
            shouldEncode |= (tournament.getCategoryResultsResets().find(*subscribedCategory) != tournament.getCategoryResultsResets().end());
            if (shouldEncode)
                document.AddMember("subscribedCategory", encodeSubscribedCategory(tournament, tournament.getCategory(*subscribedCategory), allocator), allocator);
        }
    }
    else if (subscribedPlayer.has_value()) { // encode subscribed player
        if (tournament.getErasedPlayers().find(*subscribedPlayer) != tournament.getErasedPlayers().end()) {
            document.AddMember("subscribedPlayer", rapidjson::Value(), allocator);
        }
        else {
            bool shouldEncode = false;

            shouldEncode |= (tournament.getChangedPlayers().find(*subscribedPlayer) != tournament.getChangedPlayers().end());
            shouldEncode |= (tournament.getAddedPlayers().find(*subscribedPlayer) != tournament.getAddedPlayers().end());
            shouldEncode |= (tournament.getPlayerMatchResets().find(*subscribedPlayer) != tournament.getPlayerMatchResets().end());
            if (shouldEncode)
                document.AddMember("subscribedPlayer", encodeSubscribedPlayer(tournament.getPlayer(*subscribedPlayer), allocator), allocator);
        }
    }
    else if (subscribedTatami.has_value()) {
        const auto &tatamis = tournament.getTatamis();
        if  (*subscribedTatami < tatamis.tatamiCount()) {
            if (tournament.getWebTatamiModel(*subscribedTatami).changed())
                document.AddMember("subscribedTatami", encodeSubscribedTatami(*subscribedTatami, tournament, allocator), allocator);
        }
    }

    // Identify changed matches
    std::unordered_set<CombinedId> matchIds;

    // Identify changed subscribed matches
    if (subscribedCategory.has_value() && tournament.containsCategory(*subscribedCategory)) {
        bool matchesReset = (tournament.getCategoryMatchResets().find(*subscribedCategory) != tournament.getCategoryMatchResets().end());
        for (const auto &match : tournament.getCategory(*subscribedCategory).getMatches()) {
            const auto &combinedId = match->getCombinedId();
            if (!matchesReset && tournament.getChangedMatches().find(combinedId) == tournament.getChangedMatches().end())
                continue;
            matchIds.insert(combinedId);
        }
    }
    else if (subscribedPlayer.has_value() && tournament.containsPlayer(*subscribedPlayer)) {
        const auto &player = tournament.getPlayer(*subscribedPlayer);
        bool matchesReset = (tournament.getPlayerMatchResets().find(*subscribedPlayer) != tournament.getPlayerMatchResets().end());

        for (const auto &combinedId : player.getMatches()) {
            if (!matchesReset && tournament.getChangedMatches().find(combinedId) == tournament.getChangedMatches().end())
                continue;
            matchIds.insert(combinedId);
        }
    }

    // Identify changed tatami matches
    auto tatamiCount = tournament.getTatamis().tatamiCount();
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

    // Encode matches
    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &combinedId : matchIds) {
        const auto &category = tournament.getCategory(combinedId.getCategoryId());
        const auto &match = category.getMatch(combinedId.getMatchId());
        matches.PushBack(encodeMatch(category, match, clockDiff, allocator, true), allocator);
    }
    document.AddMember("matches", matches, allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

rapidjson::Value JsonEncoder::encodePlayer(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("id", player.getId().getValue(), allocator);
    res.AddMember("firstName", encodeString(player.getFirstName(), allocator), allocator);
    res.AddMember("lastName", encodeString(player.getLastName(), allocator), allocator);

    if (player.getCountry().has_value())
        res.AddMember("country", encodeString(player.getCountry()->toString(), allocator), allocator);
    else
        res.AddMember("country", rapidjson::Value(), allocator);

    if (player.getRank().has_value())
        res.AddMember("rank", encodeString(player.getRank()->toString(), allocator), allocator);
    else
        res.AddMember("rank", rapidjson::Value(), allocator);

    // if (player.getSex().has_value())
    //     res.AddMember("sex", encodeString(player.getSex()->toString(), allocator), allocator);
    // else
    //     res.AddMember("sex", rapidjson::Value(), allocator);

    res.AddMember("club", encodeString(player.getClub(), allocator), allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeSubscribedPlayer(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res = encodePlayer(player, allocator);

    rapidjson::Value categories(rapidjson::kArrayType);
    for (const auto &categoryId : player.getCategories())
        categories.PushBack(categoryId.getValue(), allocator);
    res.AddMember("categories", categories, allocator);

    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &combinedId : player.getMatches())
        matches.PushBack(encodeCombinedId(combinedId, allocator), allocator);

    res.AddMember("matches", matches, allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("id", category.getId().getValue(), allocator);
    res.AddMember("name", encodeString(category.getName(), allocator), allocator);
    res.AddMember("matchesHidden", category.areMatchesHidden(), allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeSubscribedCategory(const TournamentStore &tournament, const CategoryStore &category, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res = encodeCategory(category, allocator);

    // Encode matches
    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &match : category.getMatches())
        matches.PushBack(encodeCombinedId(match->getCombinedId(), allocator), allocator);

    res.AddMember("matches", matches, allocator);

    // Encode players
    rapidjson::Value players(rapidjson::kArrayType);
    for (auto &playerId : category.getPlayers()) {
        rapidjson::Value val(playerId.getValue());
        players.PushBack(val, allocator);
    }

    res.AddMember("players", players, allocator);
    res.AddMember("results", encodeCategoryResults(tournament, category, allocator), allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeMatch(const CategoryStore &category, const MatchStore &match, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator, bool shouldCache) {
    auto combinedId = match.getCombinedId();

    auto it = mCachedMatches.find(combinedId);
    if (it != mCachedMatches.end()) {
        rapidjson::Value res(it->second, allocator);
        return res;
    }

    const auto &ruleset = category.getRuleset();

    rapidjson::Value res;
    res.SetObject();

    res.AddMember("combinedId", encodeCombinedId(combinedId, allocator), allocator);
    res.AddMember("bye", match.isBye(), allocator);
    res.AddMember("title", encodeString(match.getTitle(), allocator), allocator);
    res.AddMember("position", category.getMatchPosition(match.getId()), allocator);

    if (match.getWhitePlayer().has_value())
        res.AddMember("whitePlayer", match.getWhitePlayer()->getValue(), allocator);
    else
        res.AddMember("whitePlayer", rapidjson::Value(), allocator);

    if (match.getBluePlayer().has_value())
        res.AddMember("bluePlayer", match.getBluePlayer()->getValue(), allocator);
    else
        res.AddMember("bluePlayer", rapidjson::Value(), allocator);

    res.AddMember("status", encodeMatchStatus(match.getStatus(), allocator), allocator);

    res.AddMember("whiteScore", encodeMatchScore(match.getWhiteScore(), allocator), allocator);
    res.AddMember("blueScore", encodeMatchScore(match.getBlueScore(), allocator), allocator);

    res.AddMember("goldenScore", match.isGoldenScore(), allocator);

    if (match.getStatus() == MatchStatus::UNPAUSED)
        res.AddMember("resumeTime", encodeTime(match.getResumeTime(), clockDiff, allocator), allocator);
    else
        res.AddMember("resumeTime", rapidjson::Value(), allocator);

    res.AddMember("duration", encodeDuration(match.getDuration(), allocator), allocator);

    res.AddMember("normalTime", encodeDuration(category.getRuleset().getNormalTime(), allocator), allocator);

    res.AddMember("osaekomi", encodeOsaekomi(match.getOsaekomi(), clockDiff, allocator), allocator);

    std::optional<MatchStore::PlayerIndex> winner;
    if (match.getStatus() == MatchStatus::FINISHED)
        winner = ruleset.getWinner(match);
    if (winner.has_value())
        res.AddMember("winner", encodeString(winner == MatchStore::PlayerIndex::WHITE ? "WHITE" : "BLUE", allocator), allocator);
    else
        res.AddMember("winner", rapidjson::Value(), allocator);

    rapidjson::Value events(rapidjson::kArrayType);
    for (const MatchEvent &event : match.getEvents())
        events.PushBack(encodeMatchEvent(event, allocator), allocator);

    res.AddMember("events", events, allocator);

    if (shouldCache) {
        rapidjson::Document cache;
        cache.CopyFrom(res, cache.GetAllocator());
        mCachedMatches.emplace(combinedId, std::move(cache));
    }

    return res;
}

rapidjson::Value JsonEncoder::encodeMatchScore(const MatchStore::Score &score, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("ippon", score.ippon, allocator);
    res.AddMember("wazari", score.wazari, allocator);
    res.AddMember("shido", score.shido, allocator);
    res.AddMember("hansokuMake", score.hansokuMake, allocator);
    return res;
}

rapidjson::Value JsonEncoder::encodeMatchStatus(const MatchStatus &status, rapidjson::Document::AllocatorType &allocator) {
    if (status == MatchStatus::NOT_STARTED)
        return encodeString("NOT_STARTED", allocator);
    if (status == MatchStatus::PAUSED)
        return encodeString("PAUSED", allocator);
    if (status == MatchStatus::UNPAUSED)
        return encodeString("UNPAUSED", allocator);
    return encodeString("FINISHED", allocator);
}

rapidjson::Value JsonEncoder::encodeCombinedId(const CombinedId &id, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("categoryId", id.getCategoryId().getValue(), allocator);
    res.AddMember("matchId", id.getMatchId().getValue(), allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeString(const std::string &str, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value json;
    json.SetString(str.c_str(), str.size(), allocator);
    return json;
}

rapidjson::Value JsonEncoder::encodeMeta(const WebTournamentStore &tournament, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("name", encodeString(tournament.getName(), allocator), allocator);
    res.AddMember("location", encodeString(tournament.getLocation(), allocator), allocator);
    res.AddMember("date", encodeString(tournament.getDate(), allocator), allocator);
    res.AddMember("webName", encodeString(tournament.getWebName(), allocator), allocator);
    res.AddMember("tatamiCount", tournament.getTatamis().tatamiCount(), allocator);

    return res;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeTournamentSubscriptionFailMessage() {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("type", encodeString("tournamentSubscriptionFail", allocator), allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeCategorySubscriptionFailMessage() {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("type", encodeString("categorySubscriptionFail", allocator), allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeTatamiSubscriptionFailMessage() {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("type", encodeString("tatamiSubscriptionFail", allocator), allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodePlayerSubscriptionFailMessage() {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("type", encodeString("playerSubscriptionFail", allocator), allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

rapidjson::Value JsonEncoder::encodeMatchEvent(const MatchEvent &event, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    // encode type
    if (event.type == MatchEventType::IPPON)
        res.AddMember("type", encodeString("IPPON", allocator), allocator);
    else if (event.type == MatchEventType::WAZARI)
        res.AddMember("type", encodeString("WAZARI", allocator), allocator);
    else if (event.type == MatchEventType::SHIDO)
        res.AddMember("type", encodeString("SHIDO", allocator), allocator);
    else if (event.type == MatchEventType::HANSOKU_MAKE)
        res.AddMember("type", encodeString("HANSOKU_MAKE", allocator), allocator);
    else if (event.type == MatchEventType::IPPON_OSAEKOMI)
        res.AddMember("type", encodeString("IPPON_OSAEKOMI", allocator), allocator);
    else if (event.type == MatchEventType::WAZARI_OSAEKOMI)
        res.AddMember("type", encodeString("WAZARI_OSAEKOMI", allocator), allocator);
    else if (event.type == MatchEventType::CANCEL_IPPON)
        res.AddMember("type", encodeString("CANCEL_IPPON", allocator), allocator);
    else if (event.type == MatchEventType::CANCEL_WAZARI)
        res.AddMember("type", encodeString("CANCEL_WAZARI", allocator), allocator);
    else if (event.type == MatchEventType::CANCEL_SHIDO)
        res.AddMember("type", encodeString("CANCEL_SHIDO", allocator), allocator);
    else
        res.AddMember("type", encodeString("CANCEL_HANSOKU_MAKE", allocator), allocator);

    res.AddMember("playerIndex", encodeString(event.playerIndex == MatchStore::PlayerIndex::WHITE ? "WHITE" : "BLUE", allocator), allocator);
    res.AddMember("duration", encodeDuration(event.duration, allocator), allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeDuration(const std::chrono::milliseconds &duration, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res(duration.count());

    return res;
}

rapidjson::Value JsonEncoder::encodeTime(const std::chrono::milliseconds &time, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator) {
    auto sinceEpoch = time - clockDiff;
    rapidjson::Value res(sinceEpoch.count()); // unix timestamp

    return res;
}

rapidjson::Value JsonEncoder::encodeTatami(size_t index, const WebTatamiModel &model, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("index", index, allocator);

    // encode matches
    rapidjson::Value matches(rapidjson::kArrayType);
    for (auto &combinedId : model.getMatches())
        matches.PushBack(encodeCombinedId(combinedId, allocator), allocator);

    res.AddMember("matches", matches, allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeOsaekomi(const std::optional<std::pair<MatchStore::PlayerIndex, std::chrono::milliseconds>>& osaekomi, std::chrono::milliseconds clockDiff, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    if (!osaekomi.has_value())
        return res;

    res.SetObject();
    res.AddMember("player", encodeString(osaekomi->first == MatchStore::PlayerIndex::WHITE ? "WHITE" : "BLUE", allocator), allocator);
    res.AddMember("start", encodeTime(osaekomi->second, clockDiff, allocator), allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeCategoryResults(const TournamentStore &tournament, const CategoryStore &category, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res(rapidjson::kArrayType);

    const auto &drawSystem = category.getDrawSystem();
    auto results = drawSystem.getResults(tournament, category);

    for (const auto &row : results) {
        rapidjson::Value val;
        val.SetObject();

        val.AddMember("player", row.first.getValue(), allocator);

        rapidjson::Value pos;
        if (row.second.has_value())
            pos.Set(*(row.second), allocator);
        val.AddMember("pos", pos, allocator);

        res.PushBack(val, allocator);
    }

    return res;
}

rapidjson::Value JsonEncoder::encodeSubscribedTatami(size_t index, const WebTournamentStore &tournament, rapidjson::Document::AllocatorType &allocator) {
    assert(index < tournament.getTatamis().tatamiCount());
    const auto &tatami = tournament.getTatamis().at(index);
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("index", index, allocator);

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
                block.AddMember("type", encodeMatchType(matchType, allocator), allocator);
                block.AddMember("status", encodeBlockStatus(tournament.getCategory(categoryId), matchType, allocator), allocator);

                blocks.PushBack(block, allocator);
            }

            sequentialGroups.PushBack(blocks, allocator);
        }

        concurrentGroups.PushBack(sequentialGroups, allocator);
    }

    res.AddMember("blocks", concurrentGroups, allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeMatchType(MatchType matchType, rapidjson::Document::AllocatorType &allocator) {
    // encode type
    if (matchType == MatchType::FINAL)
        return encodeString("FINAL", allocator);
    else
        return encodeString("ELIMINATION", allocator);
}

rapidjson::Value JsonEncoder::encodeBlockStatus(const CategoryStore &category, MatchType matchType, rapidjson::Document::AllocatorType &allocator) {
    const auto &status = category.getStatus(matchType);

    if (status.notStartedMatches > 0 && status.startedMatches == 0 && status.finishedMatches == 0)
        return encodeString("NOT_STARTED", allocator);
    else if (status.startedMatches > 0 || status.notStartedMatches > 0)
        return encodeString("STARTED", allocator);
    else
        return encodeString("FINISHED", allocator);
}

rapidjson::Value JsonEncoder::encodeTournamentListing(const TournamentListing &tournament, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("name", encodeString(tournament.name, allocator), allocator);
    res.AddMember("webName", encodeString(tournament.webName, allocator), allocator);
    res.AddMember("location", encodeString(tournament.location, allocator), allocator);
    res.AddMember("date", encodeString(tournament.date, allocator), allocator);

    return res;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeTournamentListingMessage(const std::vector<TournamentListing> &pastTournamentsListing, const std::vector<TournamentListing> &upcomingTournamentsListing) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("type", encodeString("tournamentListing", allocator), allocator);

    // Past tournaments
    rapidjson::Value pastTournaments(rapidjson::kArrayType);
    for (const auto &tournament : pastTournamentsListing)
        pastTournaments.PushBack(encodeTournamentListing(tournament, allocator), allocator);
    document.AddMember("pastTournaments", pastTournaments, allocator);

    // Upcoming tournaments
    rapidjson::Value upcomingTournaments(rapidjson::kArrayType);
    for (const auto &tournament : upcomingTournamentsListing)
        upcomingTournaments.PushBack(encodeTournamentListing(tournament, allocator), allocator);
    document.AddMember("upcomingTournaments", upcomingTournaments, allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeTournamentListingFailMessage() {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("type", encodeString("tournamentListingFail", allocator), allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeClockMessage(std::chrono::milliseconds clock) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("type", encodeString("clock", allocator), allocator);
    document.AddMember("clock", clock.count(), allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return buffer;
}

