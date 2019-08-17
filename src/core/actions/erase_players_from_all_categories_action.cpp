#include <set>

#include "core/actions/draw_categories_action.hpp"
#include "core/actions/erase_players_from_all_categories_action.hpp"
#include "core/actions/erase_players_from_category_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/id.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tournament_store.hpp"

ErasePlayersFromAllCategoriesAction::ErasePlayersFromAllCategoriesAction(const std::vector<PlayerId> &playerIds)
    : ErasePlayersFromAllCategoriesAction(playerIds, getSeed())
{}

ErasePlayersFromAllCategoriesAction::ErasePlayersFromAllCategoriesAction(const std::vector<PlayerId> &playerIds, unsigned int seed)
    : mPlayerIds(playerIds)
    , mSeed(seed)
{}

std::unique_ptr<Action> ErasePlayersFromAllCategoriesAction::freshClone() const {
    return std::make_unique<ErasePlayersFromAllCategoriesAction>(mPlayerIds, mSeed);
}

void ErasePlayersFromAllCategoriesAction::redoImpl(TournamentStore & tournament) {
    std::vector<PlayerId> playerIds;
    std::set<CategoryId> categoryIds;

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        playerIds.push_back(playerId);
        const PlayerStore &player = tournament.getPlayer(playerId);
        const auto &playerCategories = player.getCategories();
        categoryIds.insert(playerCategories.begin(), playerCategories.end());
    }

    for (auto categoryId : categoryIds) {
        auto action = std::make_unique<ErasePlayersFromCategoryAction>(categoryId, playerIds, mSeed);
        action->redo(tournament);
        mActions.push(std::move(action));
    }
}

void ErasePlayersFromAllCategoriesAction::undoImpl(TournamentStore & tournament) {
    while (!mActions.empty()) {
        mActions.top()->undo(tournament);
        mActions.pop();
    }
}

std::string ErasePlayersFromAllCategoriesAction::getDescription() const {
    return "Erase players from all categories";
}

