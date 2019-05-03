#include "core/actions/set_match_player_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"

SetMatchPlayerAction::SetMatchPlayerAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::optional<PlayerId> playerId)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
    , mPlayerIndex(playerIndex)
    , mPlayerId(playerId)
{}

void SetMatchPlayerAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    if (mPlayerId.has_value() && !tournament.containsPlayer(*mPlayerId))
        return;

    mOldPlayerId = match.getPlayer(mPlayerIndex);
    match.setPlayer(mPlayerIndex, mPlayerId);

    if (mPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mPlayerId);
        player.addMatch(mCategoryId, mMatchId);
    }

    if (mOldPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mOldPlayerId);
        player.eraseMatch(mCategoryId, mMatchId);
    }

    tournament.changeMatches(mCategoryId, {mMatchId});
    if (mPlayerId.has_value())
        tournament.addMatchesToPlayer(*mPlayerId, {std::make_pair(mCategoryId, mMatchId)});
    if (mOldPlayerId.has_value())
        tournament.eraseMatchesFromPlayer(*mPlayerId, {std::make_pair(mCategoryId, mMatchId)});
}

void SetMatchPlayerAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    if (mPlayerId.has_value() && !tournament.containsPlayer(*mPlayerId))
        return;

    match.setPlayer(mPlayerIndex, mOldPlayerId);

    if (mOldPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mOldPlayerId);
        player.addMatch(mCategoryId, mMatchId);
    }

    if (mPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mPlayerId);
        player.eraseMatch(mCategoryId, mMatchId);
    }

    tournament.changeMatches(mCategoryId, {mMatchId});
    if (mPlayerId.has_value())
        tournament.eraseMatchesFromPlayer(*mPlayerId, {std::make_pair(mCategoryId, mMatchId)});
    if (mOldPlayerId.has_value())
        tournament.addMatchesToPlayer(*mPlayerId, {std::make_pair(mCategoryId, mMatchId)});
}

std::unique_ptr<Action> SetMatchPlayerAction::freshClone() const {
    return std::make_unique<SetMatchPlayerAction>(mCategoryId, mMatchId, mPlayerIndex, mPlayerId);
}

std::string SetMatchPlayerAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Set white match player";
    return "Set blue match player";
}
