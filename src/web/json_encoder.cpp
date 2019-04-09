#include <rapidjson/writer.h>

#include "core/draw_systems/draw_system.hpp"
#include "core/log.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "web/json_encoder.hpp"
#include "web/web_tournament_store.hpp"

// TODO: Don't trust state of tournament to be correct
JsonBuffer::JsonBuffer() {

}

boost::asio::const_buffer JsonBuffer::getBuffer() const {
    return boost::asio::buffer(mStringBuffer.GetString(), mStringBuffer.GetSize());
}

rapidjson::StringBuffer& JsonBuffer::getStringBuffer() {
    return mStringBuffer;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeTournamentSubscriptionMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("messageType", encodeString("tournamentSubscription", allocator), allocator);

    // Tournament meta data field
    document.AddMember("tournament", encodeMeta(tournament, allocator), allocator);

    // categories field
    rapidjson::Value categories(rapidjson::kArrayType);
    for (const auto &p : tournament.getCategories())
        categories.PushBack(encodeCategory(*(p.second), allocator), allocator);
    document.AddMember("categories", categories, allocator);

    // subscribed category field
    if (subscribedCategory.has_value() && tournament.containsCategory(*subscribedCategory))
        document.AddMember("subscribedCategory", encodeSubscribedCategory(tournament.getCategory(*subscribedCategory), allocator), allocator);
    else
        document.AddMember("subscribedCategory", rapidjson::Value(), allocator);

    // players field
    rapidjson::Value players(rapidjson::kArrayType);
    for (const auto &p : tournament.getPlayers())
        players.PushBack(encodePlayer(*(p.second), allocator), allocator);
    document.AddMember("players", players, allocator);

    // subscribed category field
    if (subscribedPlayer.has_value() && tournament.containsPlayer(*subscribedPlayer))
        document.AddMember("subscribedPlayer", encodeSubscribedPlayer(tournament.getPlayer(*subscribedPlayer), allocator), allocator);
    else
        document.AddMember("subscribedPlayer", rapidjson::Value(), allocator);

    // Encode matches
    // TODO: Encode tatami matches
    std::unordered_set<std::pair<CategoryId, MatchId>> matchIds;

    if (subscribedCategory.has_value() && tournament.containsCategory(*subscribedCategory)) {
        for (const auto &match : tournament.getCategory(*subscribedCategory).getMatches())
            matchIds.insert(match->getCombinedId());
    }
    else if (subscribedPlayer.has_value() && tournament.containsPlayer(*subscribedPlayer)) {
        const auto &player = tournament.getPlayer(*subscribedPlayer);

        for (const auto &combinedId : player.getMatches())
            matchIds.insert(combinedId);
    }

    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &combinedId : matchIds) {
        const auto &match = tournament.getCategory(combinedId.first).getMatch(combinedId.second);
        matches.PushBack(encodeMatch(match, allocator), allocator);
    }

    document.AddMember("matches", matches, allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return std::move(buffer);
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeCategorySubscriptionMessage(const WebTournamentStore &tournament, const CategoryStore &category) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("messageType", encodeString("categorySubscription", allocator), allocator);

    // subscribed category field
    document.AddMember("subscribedCategory", encodeSubscribedCategory(category, allocator), allocator);

    // matches
    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &match : category.getMatches())
        matches.PushBack(encodeMatch(*match, allocator), allocator);
    document.AddMember("matches", matches, allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return std::move(buffer);
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodePlayerSubscriptionMessage(const WebTournamentStore &tournament, const PlayerStore &player) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("messageType", encodeString("playerSubscription", allocator), allocator);

    // subscribed category field
    document.AddMember("subscribedPlayer", encodeSubscribedPlayer(player, allocator), allocator);

    // matches
    rapidjson::Value matches(rapidjson::kArrayType);
    log_debug().field("size", player.getMatches().size()).msg("Listing matches");
    for (auto combinedId : player.getMatches()) {
        const auto &match = tournament.getCategory(combinedId.first).getMatch(combinedId.second);
        matches.PushBack(encodeMatch(match, allocator), allocator);
    }
    document.AddMember("matches", matches, allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return std::move(buffer);
}

bool JsonEncoder::hasTournamentChanges(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer) {
    // check tournament
    if (tournament.tournamentChanged())
        return true;

    // check players
    if (!tournament.getChangedPlayers().empty())
        return true;
    if (!tournament.getAddedPlayers().empty())
        return true;
    if (!tournament.getErasedPlayers().empty())
        return true;

    // check categories
    if (!tournament.getChangedCategories().empty())
        return true;
    if (!tournament.getAddedCategories().empty())
        return true;
    if (!tournament.getErasedCategories().empty())
        return true;

    // check matches
    if (subscribedCategory.has_value() && tournament.containsCategory(*subscribedCategory)) {
        if (tournament.getCategoryMatchResets().find(*subscribedCategory) != tournament.getCategoryMatchResets().end())
            return true;

        for (const auto &match : tournament.getCategory(*subscribedCategory).getMatches()) {
            const auto &combinedId = match->getCombinedId();
            if (tournament.getChangedMatches().find(combinedId) != tournament.getChangedMatches().end())
                return true;
        }
    }
    else if (subscribedPlayer.has_value() && tournament.containsPlayer(*subscribedPlayer)) {
        if (tournament.getPlayerMatchResets().find(*subscribedPlayer) != tournament.getPlayerMatchResets().end())
            return true;

        const auto &player = tournament.getPlayer(*subscribedPlayer);
        for (const auto &combinedId : player.getMatches()) {
            if (tournament.getChangedMatches().find(combinedId) != tournament.getChangedMatches().end())
                return true;
        }
    }

    return false;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeTournamentChangesMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("messageType", encodeString("tournamentChanges", allocator), allocator);

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

    if (subscribedCategory.has_value()) { // encode subscribed category
        if (tournament.getErasedCategories().find(*subscribedCategory) != tournament.getErasedCategories().end()) {
            document.AddMember("subscribedCategory", rapidjson::Value(), allocator);
        }
        else {
            bool shouldEncode = false;

            shouldEncode |= (tournament.getChangedCategories().find(*subscribedCategory) != tournament.getChangedCategories().end());
            shouldEncode |= (tournament.getAddedCategories().find(*subscribedCategory) != tournament.getAddedCategories().end());
            shouldEncode |= (tournament.getCategoryMatchResets().find(*subscribedCategory) != tournament.getCategoryMatchResets().end());
            if (shouldEncode)
                document.AddMember("subscribedCategory", encodeSubscribedCategory(tournament.getCategory(*subscribedCategory), allocator), allocator);
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

    // // encode matches
    // TODO: Encode tatami matches
    std::unordered_set<std::pair<CategoryId, MatchId>> matchIds;

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

    rapidjson::Value matches(rapidjson::kArrayType);
    for (const auto &combinedId : matchIds) {
        const auto &match = tournament.getCategory(combinedId.first).getMatch(combinedId.second);
        matches.PushBack(encodeMatch(match, allocator), allocator);
    }
    document.AddMember("matches", matches, allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return std::move(buffer);
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

    if (player.getSex().has_value())
        res.AddMember("sex", encodeString(player.getSex()->toString(), allocator), allocator);
    else
        res.AddMember("sex", rapidjson::Value(), allocator);

    res.AddMember("club", encodeString(player.getClub(), allocator), allocator);

    rapidjson::Value categories(rapidjson::kArrayType);
    for (const auto &categoryId : player.getCategories())
        categories.PushBack(categoryId.getValue(), allocator);
    res.AddMember("categories", categories, allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeSubscribedPlayer(const PlayerStore &player, rapidjson::Document::AllocatorType &allocator) {
    // TODO: Implement
    rapidjson::Value res = encodePlayer(player, allocator);

    rapidjson::Value matches(rapidjson::kArrayType);

    for (auto combinedId : player.getMatches()) {
        rapidjson::Value match;
        match.SetObject();
        match.AddMember("category", combinedId.first.getValue(), allocator);
        match.AddMember("match", combinedId.second.getValue(), allocator);
        matches.PushBack(match, allocator);
    }

    res.AddMember("matches", matches, allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("id", category.getId().getValue(), allocator);
    res.AddMember("name", encodeString(category.getName(), allocator), allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeSubscribedCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res = encodeCategory(category, allocator);

    rapidjson::Value matches(rapidjson::kArrayType);

    for (const auto &match : category.getMatches()) {
        rapidjson::Value val(match->getId().getValue());
        matches.PushBack(val, allocator);
    }

    res.AddMember("matches", matches, allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeMatch(const MatchStore &match, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("id", match.getId().getValue(), allocator);
    res.AddMember("isBye", match.isBye(), allocator);
    if (match.getWhitePlayer().has_value())
        res.AddMember("whitePlayer", match.getWhitePlayer()->getValue(), allocator);
    else
        res.AddMember("whitePlayer", rapidjson::Value(), allocator);

    if (match.getBluePlayer().has_value())
        res.AddMember("bluePlayer", match.getBluePlayer()->getValue(), allocator);
    else
        res.AddMember("bluePlayer", rapidjson::Value(), allocator);

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

    log_debug().field("name", tournament.getName()).msg("Encoding meta");
    res.AddMember("name", encodeString(tournament.getName(), allocator), allocator);
    res.AddMember("webName", encodeString(tournament.getWebName(), allocator), allocator);

    return res;
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeTournamentSubscriptionFailMessage() {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("messageType", encodeString("tournamentSubscriptionFail", allocator), allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return std::move(buffer);
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeCategorySubscriptionFailMessage() {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("messageType", encodeString("categorySubscriptionFail", allocator), allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return std::move(buffer);
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodePlayerSubscriptionFailMessage() {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("messageType", encodeString("playerSubscriptionFail", allocator), allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return std::move(buffer);
}

