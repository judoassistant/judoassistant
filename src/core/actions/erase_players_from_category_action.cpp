#include "core/actions/draw_categories_action.hpp"
#include "core/actions/erase_players_from_category_action.hpp"
#include "core/actions/change_categories_draw_system_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/preferences_store.hpp"

ErasePlayersFromCategoryAction::ErasePlayersFromCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds)
    : ErasePlayersFromCategoryAction(categoryId, playerIds, getSeed())
{}

ErasePlayersFromCategoryAction::ErasePlayersFromCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds, unsigned int seed)
    : mCategoryId(categoryId)
    , mPlayerIds(playerIds)
    , mSeed(seed)
{}

std::unique_ptr<Action> ErasePlayersFromCategoryAction::freshClone() const {
    return std::make_unique<ErasePlayersFromCategoryAction>(mCategoryId, mPlayerIds, mSeed);
}

void ErasePlayersFromCategoryAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (!player.containsCategory(mCategoryId))
            continue;

        mErasedPlayerIds.push_back(playerId);
        player.eraseCategory(mCategoryId);
        category.erasePlayer(playerId);
    }

    tournament.erasePlayersFromCategory(mCategoryId, mErasedPlayerIds);

    if (!mErasedPlayerIds.empty()) {
        const auto categoryPlayerCount = category.getPlayers().size();
        const DrawSystemIdentifier preferredDrawSystem = tournament.getPreferences().getPreferredDrawSystem(categoryPlayerCount);
        const bool shouldChangeDrawSystem = category.getDrawSystem().getIdentifier() != preferredDrawSystem;

        if (shouldChangeDrawSystem)
            mDrawAction = std::make_unique<ChangeCategoriesDrawSystemAction>(std::vector<CategoryId>{mCategoryId}, preferredDrawSystem, mSeed);
        else
            mDrawAction = std::make_unique<DrawCategoriesAction>(std::vector<CategoryId>{mCategoryId}, mSeed);
        mDrawAction->redo(tournament);
    }
}

void ErasePlayersFromCategoryAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);

    tournament.addPlayersToCategory(mCategoryId, mErasedPlayerIds);

    for (auto playerId : mErasedPlayerIds) {
        PlayerStore & player = tournament.getPlayer(playerId);
        player.addCategory(mCategoryId);
        category.addPlayer(playerId);
    }

    if (mDrawAction != nullptr) {
        mDrawAction->undo(tournament);
        mDrawAction.reset();
    }

    mErasedPlayerIds.clear();
}

std::string ErasePlayersFromCategoryAction::getDescription() const {
    return "Erase players from category";
}

bool ErasePlayersFromCategoryAction::doesRequireConfirmation(const TournamentStore &tournament) const {
    bool playersExist = false;
    for (auto playerId : mPlayerIds) {
        if (tournament.containsPlayer(playerId)) {
            playersExist = true;
            break;
        }
    }

    if (!playersExist)
        return false;

    const CategoryStore & category = tournament.getCategory(mCategoryId);
    return category.isStarted();
}

