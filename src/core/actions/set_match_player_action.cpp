#include "core/actions/set_match_player_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"

SetMatchPlayerAction::SetMatchPlayerAction(CombinedId combinedId, MatchStore::PlayerIndex playerIndex, std::optional<PlayerId> playerId)
    : mCombinedId(combinedId)
    , mPlayerIndex(playerIndex)
    , mPlayerId(playerId)
{}

void SetMatchPlayerAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;
    auto &match = category.getMatch(mCombinedId.getMatchId());

    if (mPlayerId.has_value() && !tournament.containsPlayer(*mPlayerId))
        return;

    mOldPlayerId = match.getPlayer(mPlayerIndex);
    match.setPlayer(mPlayerIndex, mPlayerId);

    if (mPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mPlayerId);
        player.addMatch(mCombinedId);
    }

    if (mOldPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mOldPlayerId);
        player.eraseMatch(mCombinedId);
    }

    tournament.changeMatches(mCombinedId.getCategoryId(), {mCombinedId.getMatchId()});
    if (mPlayerId.has_value())
        tournament.addMatchesToPlayer(*mPlayerId, {mCombinedId});
    if (mOldPlayerId.has_value())
        tournament.eraseMatchesFromPlayer(*mPlayerId, {mCombinedId});
}

void SetMatchPlayerAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;
    auto &match = category.getMatch(mCombinedId.getMatchId());

    if (mPlayerId.has_value() && !tournament.containsPlayer(*mPlayerId))
        return;

    match.setPlayer(mPlayerIndex, mOldPlayerId);

    if (mOldPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mOldPlayerId);
        player.addMatch(mCombinedId);
    }

    if (mPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mPlayerId);
        player.eraseMatch(mCombinedId);
    }

    tournament.changeMatches(mCombinedId.getCategoryId(), {mCombinedId.getMatchId()});
    if (mPlayerId.has_value())
        tournament.eraseMatchesFromPlayer(*mPlayerId, {mCombinedId});
    if (mOldPlayerId.has_value())
        tournament.addMatchesToPlayer(*mPlayerId, {mCombinedId});
}

std::unique_ptr<Action> SetMatchPlayerAction::freshClone() const {
    return std::make_unique<SetMatchPlayerAction>(mCombinedId, mPlayerIndex, mPlayerId);
}

std::string SetMatchPlayerAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Set white match player";
    return "Set blue match player";
}

