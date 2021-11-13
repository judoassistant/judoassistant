#include "core/actions/add_players_to_category_action.hpp"
#include "core/actions/change_categories_draw_system_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/actions/set_tatami_location_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/preferences_store.hpp"
#include "core/stores/tournament_store.hpp"

AddPlayersToCategoryAction::AddPlayersToCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds)
    : AddPlayersToCategoryAction(categoryId, playerIds, getSeed())
{}

AddPlayersToCategoryAction::AddPlayersToCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds, unsigned int seed)
    : mCategoryId(categoryId)
    , mPlayerIds(playerIds)
    , mSeed(seed)
{}

std::unique_ptr<Action> AddPlayersToCategoryAction::freshClone() const {
    return std::make_unique<AddPlayersToCategoryAction>(mCategoryId, mPlayerIds, mSeed);
}

void AddPlayersToCategoryAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;
    CategoryStore & category = tournament.getCategory(mCategoryId);

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (player.containsCategory(mCategoryId))
            continue;

        mAddedPlayerIds.push_back(playerId);
        player.addCategory(mCategoryId);
        category.addPlayer(playerId);
    }

    if (!mAddedPlayerIds.empty()) {
        const auto categoryPlayerCount = category.getPlayers().size();
        const DrawSystemIdentifier preferredDrawSystem = tournament.getPreferences().getPreferredDrawSystem(categoryPlayerCount);
        const bool shouldChangeDrawSystem = category.getDrawSystem().getIdentifier() != preferredDrawSystem;

        if (shouldChangeDrawSystem)
            mDrawAction = std::make_unique<ChangeCategoriesDrawSystemAction>(std::vector<CategoryId>{mCategoryId}, preferredDrawSystem, mSeed);
        else
            mDrawAction = std::make_unique<DrawCategoriesAction>(std::vector<CategoryId>{mCategoryId}, mSeed);
        mDrawAction->redo(tournament);
    }

    tournament.addPlayersToCategory(mCategoryId, mAddedPlayerIds);
}

void AddPlayersToCategoryAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);
    for (auto playerId : mAddedPlayerIds) {
        PlayerStore & player = tournament.getPlayer(playerId);
        player.eraseCategory(mCategoryId);
        category.erasePlayer(playerId);
    }

    if (mDrawAction != nullptr) {
        mDrawAction->undo(tournament);
        mDrawAction.reset();
    }

    tournament.erasePlayersFromCategory(mCategoryId, mAddedPlayerIds);

    mAddedPlayerIds.clear();
}

std::string AddPlayersToCategoryAction::getDescription() const {
    return "Add players to category";
}

