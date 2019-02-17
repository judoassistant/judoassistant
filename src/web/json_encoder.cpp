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

std::unique_ptr<JsonBuffer> JsonEncoder::encodeSyncMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("sync", true, allocator);

    // Tournament meta data field
    document.AddMember("tournament", encodeMeta(tournament, allocator), allocator);

    // deletedCategories field
    rapidjson::Value deletedCategories(rapidjson::kArrayType);
    document.AddMember("deletedCategories", deletedCategories, allocator);

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

    // deletedPlayers field
    rapidjson::Value deletedPlayers(rapidjson::kArrayType);
    document.AddMember("deletedPlayers", deletedPlayers, allocator);

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

    // matches

    rapidjson::Value matches(rapidjson::kArrayType);
    if (subscribedCategory.has_value() && tournament.containsCategory(*subscribedCategory)) {
        for (const auto &match : tournament.getCategory(*subscribedCategory).getMatches())
            matches.PushBack(encodeMatch(*match, allocator), allocator);
    }
    else if (subscribedPlayer.has_value() && tournament.containsPlayer(*subscribedPlayer)) {
        const auto &player = tournament.getPlayer(*subscribedPlayer);

        for (const auto &combinedId : player.getMatches()) {
            const auto &match = tournament.getCategory(combinedId.first).getMatch(combinedId.second);
            matches.PushBack(encodeMatch(match, allocator), allocator);
        }
    }
    document.AddMember("matches", matches, allocator);

    auto buffer = std::make_unique<JsonBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer->getStringBuffer());
    document.Accept(writer);

    return std::move(buffer);
}

std::unique_ptr<JsonBuffer> JsonEncoder::encodeSubscribeCategoryMessage(const WebTournamentStore &tournament, const CategoryStore &category) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("sync", false, allocator);

    // Tournament meta data field
    document.AddMember("tournament", rapidjson::Value(), allocator);

    // deletedCategories field
    rapidjson::Value deletedCategories(rapidjson::kArrayType);
    document.AddMember("deletedCategories", deletedCategories, allocator);

    // categories field
    rapidjson::Value categories(rapidjson::kArrayType);
    document.AddMember("categories", categories, allocator);

    // subscribed category field
    document.AddMember("subscribedCategory", encodeSubscribedCategory(category, allocator), allocator);

    // deletedPlayers field
    rapidjson::Value deletedPlayers(rapidjson::kArrayType);
    document.AddMember("deletedPlayers", deletedPlayers, allocator);

    // players field
    rapidjson::Value players(rapidjson::kArrayType);
    document.AddMember("players", players, allocator);

    // subscribed category field
    document.AddMember("subscribedPlayer", rapidjson::Value(), allocator);

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

std::unique_ptr<JsonBuffer> JsonEncoder::encodeSubscribePlayerMessage(const WebTournamentStore &tournament, const PlayerStore &player) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    document.AddMember("sync", false, allocator);

    // Tournament meta data field
    document.AddMember("tournament", rapidjson::Value(), allocator);

    // deletedCategories field
    rapidjson::Value deletedCategories(rapidjson::kArrayType);
    document.AddMember("deletedCategories", deletedCategories, allocator);

    // categories field
    rapidjson::Value categories(rapidjson::kArrayType);
    document.AddMember("categories", categories, allocator);

    // subscribed category field
    document.AddMember("subscribedCategory", rapidjson::Value(), allocator);

    // deletedPlayers field
    rapidjson::Value deletedPlayers(rapidjson::kArrayType);
    document.AddMember("deletedPlayers", deletedPlayers, allocator);

    // players field
    rapidjson::Value players(rapidjson::kArrayType);
    document.AddMember("players", players, allocator);

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

std::unique_ptr<JsonBuffer> JsonEncoder::encodeChangesMessage(const WebTournamentStore &tournament, std::optional<CategoryId> subscribedCategory, std::optional<PlayerId> subscribedPlayer) {
    // TODO: Implement
    rapidjson::Document document;
    document.SetObject();
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
    return encodePlayer(player, allocator);
}

rapidjson::Value JsonEncoder::encodeCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value res;
    res.SetObject();

    res.AddMember("id", category.getId().getValue(), allocator);
    res.AddMember("name", encodeString(category.getName(), allocator), allocator);

    return res;
}

rapidjson::Value JsonEncoder::encodeSubscribedCategory(const CategoryStore &category, rapidjson::Document::AllocatorType &allocator) {
    // TODO: Implement
    return encodeCategory(category, allocator);
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

    res.AddMember("name", encodeString(tournament.getName(), allocator), allocator);
    res.AddMember("webName", encodeString(tournament.getWebName(), allocator), allocator);

    return res;
}

