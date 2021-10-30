#include "core/actions/change_players_blue_judogi_hint_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/random.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersBlueJudogiHintAction::ChangePlayersBlueJudogiHintAction(std::vector<PlayerId> playerIds, bool value)
    : mPlayerIds(playerIds)
    , mValue(value)
    , mSeed(getSeed())
{}

ChangePlayersBlueJudogiHintAction::ChangePlayersBlueJudogiHintAction(std::vector<PlayerId> playerIds, bool value, size_t seed)
    : mPlayerIds(playerIds)
    , mValue(value)
    , mSeed(seed)
{}

void ChangePlayersBlueJudogiHintAction::redoImpl(TournamentStore & tournament) {
    std::set<CategoryId> categoryIds;

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (player.getBlueJudogiHint() == mValue)
            continue;

        mChangedPlayers.push_back(playerId);
        mOldValues.push_back(player.getBlueJudogiHint());
        categoryIds.insert(player.getCategories().begin(), player.getCategories().end());
        player.setBlueJudogiHint(mValue);
    }

    if (!categoryIds.empty()) {
        mDrawAction = std::make_unique<DrawCategoriesAction>(std::vector(categoryIds.begin(), categoryIds.end()), mSeed);
        mDrawAction->redo(tournament);
    }
    else {
        mDrawAction.reset();
    }
    tournament.changePlayers(mChangedPlayers);
}

void ChangePlayersBlueJudogiHintAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mChangedPlayers) {
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setBlueJudogiHint(*i);

        std::advance(i, 1);
    }

    if (mDrawAction) {
        mDrawAction->undo(tournament);
        mDrawAction.reset();
    }

    tournament.changePlayers(mChangedPlayers);
    mChangedPlayers.clear();
    mOldValues.clear();
}

std::string ChangePlayersBlueJudogiHintAction::getDescription() const {
    return "Change players blue judogi hint";
}

std::unique_ptr<Action> ChangePlayersBlueJudogiHintAction::freshClone() const {
    return std::make_unique<ChangePlayersBlueJudogiHintAction>(mPlayerIds, mValue, mSeed);
}

