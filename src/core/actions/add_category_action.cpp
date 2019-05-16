#include "core/actions/add_category_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"

AddCategoryAction::AddCategoryAction(TournamentStore & tournament, const std::string &name, RulesetIdentifier ruleset, DrawSystemIdentifier drawSystem)
    : AddCategoryAction(CategoryId::generate(tournament), name, ruleset, drawSystem)
{}

AddCategoryAction::AddCategoryAction(CategoryId id, const std::string &name, RulesetIdentifier ruleset, DrawSystemIdentifier drawSystem)
    : mId(id)
    , mName(name)
    , mRuleset(ruleset)
    , mDrawSystem(drawSystem)
{}

std::unique_ptr<Action> AddCategoryAction::freshClone() const {
    return std::make_unique<AddCategoryAction>(mId, mName, mRuleset, mDrawSystem);
}

CategoryId AddCategoryAction::getId() const {
    return mId;
}

void AddCategoryAction::redoImpl(TournamentStore & tournament) {
    if (tournament.containsCategory(mId))
        throw ActionExecutionException("Failed to redo AddCategoryAction. Category already exists.");

    auto ruleset = Ruleset::getRuleset(mRuleset);
    auto drawSystem = DrawSystem::getDrawSystem(mDrawSystem);

    tournament.beginAddCategories({mId});
    tournament.addCategory(std::make_unique<CategoryStore>(mId, mName, std::move(ruleset), std::move(drawSystem)));
    tournament.endAddCategories({mId});
}

void AddCategoryAction::undoImpl(TournamentStore & tournament) {
    tournament.beginEraseCategories({mId});
    tournament.eraseCategory(mId);
    tournament.endEraseCategories({mId});
}

std::string AddCategoryAction::getDescription() const {
    return "Add category";
}

