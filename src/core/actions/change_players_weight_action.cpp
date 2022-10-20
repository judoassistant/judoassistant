#include "core/actions/change_players_weight_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersWeightAction::ChangePlayersWeightAction(std::vector<PlayerId> playerIds, std::optional<PlayerWeight> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersWeightAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (player.getWeight() == mValue)
            continue;
        mChangedPlayers.push_back(playerId);
        mOldValues.push_back(player.getWeight());
        player.setWeight(mValue);
    }

    mChangedCategories = getAffectedCategories(tournament);
    for (const CategoryId categoryId : mChangedCategories) {
        const auto &category = tournament.getCategory(categoryId);
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);

        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }
    }

    tournament.changePlayers(mChangedPlayers);
    tournament.resetCategoryResults(mChangedCategories);
}

void ChangePlayersWeightAction::undoImpl(TournamentStore & tournament) {
    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    auto i = mOldValues.begin();
    for (auto playerId : mChangedPlayers) {
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setWeight(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mChangedPlayers);
    tournament.resetCategoryResults(mChangedCategories);

    mChangedPlayers.clear();
    mChangedCategories.clear();
    mOldValues.clear();
}

std::vector<CategoryId> ChangePlayersWeightAction::getAffectedCategories(TournamentStore &tournament) {
    std::set<CategoryId> affectedCategories;
    for (const PlayerId playerId : mChangedPlayers) {
        const auto player = tournament.getPlayer(playerId);
        const auto &playerCategories = player.getCategories();
        affectedCategories.insert(playerCategories.begin(), playerCategories.end());
    }

    return std::vector(affectedCategories.begin(), affectedCategories.end());
}

std::string ChangePlayersWeightAction::getDescription() const {
    return "Change players weight";
}

std::unique_ptr<Action> ChangePlayersWeightAction::freshClone() const {
    return std::make_unique<ChangePlayersWeightAction>(mPlayerIds, mValue);
}
