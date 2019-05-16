#include "core/actions/add_category_action.hpp"
#include "core/actions/add_category_with_players_action.hpp"
#include "core/actions/add_players_to_category_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"

AddCategoryWithPlayersAction::AddCategoryWithPlayersAction(TournamentStore & tournament, const std::string &name, RulesetIdentifier ruleset, DrawSystemIdentifier drawSystem, const std::vector<PlayerId> &playerIds)
    : AddCategoryWithPlayersAction(CategoryId::generate(tournament), name, ruleset, drawSystem, playerIds, getSeed())
{}

AddCategoryWithPlayersAction::AddCategoryWithPlayersAction(CategoryId id, const std::string &name, RulesetIdentifier ruleset, DrawSystemIdentifier drawSystem, const std::vector<PlayerId> &playerIds, unsigned int seed)
    : mId(id)
    , mName(name)
    , mRuleset(ruleset)
    , mDrawSystem(drawSystem)
    , mPlayerIds(playerIds)
    , mSeed(seed)
{}

std::unique_ptr<Action> AddCategoryWithPlayersAction::freshClone() const {
    return std::make_unique<AddCategoryWithPlayersAction>(mId, mName, mRuleset, mDrawSystem, mPlayerIds, mSeed);
}

void AddCategoryWithPlayersAction::redoImpl(TournamentStore & tournament) {
    mCategoryAction = std::make_unique<AddCategoryAction>(mId, mName, mRuleset, mDrawSystem);
    mCategoryAction->redo(tournament);

    if (!mPlayerIds.empty()) {
        mPlayersAction = std::make_unique<AddPlayersToCategoryAction>(mId, mPlayerIds, mSeed);
        mPlayersAction->redo(tournament);
    }
}

void AddCategoryWithPlayersAction::undoImpl(TournamentStore & tournament) {
    if (!mPlayerIds.empty()) {
        mPlayersAction->undo(tournament);
        mPlayersAction.reset();
    }

    mCategoryAction->undo(tournament);
    mCategoryAction.reset();
}

std::string AddCategoryWithPlayersAction::getDescription() const {
    if (mPlayerIds.empty())
        return "Add category";
    return "Add category with players";
}

