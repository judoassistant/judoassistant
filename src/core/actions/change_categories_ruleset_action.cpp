#include "core/actions/change_categories_ruleset_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/id.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"

ChangeCategoriesRulesetAction::ChangeCategoriesRulesetAction(std::vector<CategoryId> categoryIds, RulesetIdentifier ruleset)
    : ChangeCategoriesRulesetAction(categoryIds, ruleset, getSeed())
{}

ChangeCategoriesRulesetAction::ChangeCategoriesRulesetAction(std::vector<CategoryId> categoryIds, RulesetIdentifier ruleset, unsigned int seed)
    : mCategoryIds(categoryIds)
    , mRuleset(ruleset)
    , mSeed(seed)
{}

std::unique_ptr<Action> ChangeCategoriesRulesetAction::freshClone() const {
    return std::make_unique<ChangeCategoriesRulesetAction>(mCategoryIds, mRuleset, mSeed);
}

void ChangeCategoriesRulesetAction::redoImpl(TournamentStore & tournament) {
    auto ruleset = Ruleset::getRuleset(mRuleset);

    std::vector<CategoryId> categoryIds;
    for (auto categoryId : mCategoryIds) {
        if (tournament.containsCategory(categoryId))
            categoryIds.push_back(categoryId);
    }

    for (auto categoryId : categoryIds) {
        CategoryStore & category = tournament.getCategory(categoryId);

        mOldRulesets.push_back(category.setRuleset(ruleset->clone()));
    }

    mDrawAction = std::make_unique<DrawCategoriesAction>(categoryIds, mSeed);
    mDrawAction->redo(tournament);

    tournament.changeCategories(categoryIds);
}

void ChangeCategoriesRulesetAction::undoImpl(TournamentStore & tournament) {
    std::vector<CategoryId> categoryIds;
    for (auto categoryId : mCategoryIds) {
        if (tournament.containsCategory(categoryId))
            categoryIds.push_back(categoryId);
    }

    assert(categoryIds.size() == mOldRulesets.size());

    mDrawAction->undo(tournament);
    mDrawAction.reset();

    for (auto i = categoryIds.rbegin(); i != categoryIds.rend(); ++i) {
        auto categoryId = *i;

        CategoryStore & category = tournament.getCategory(categoryId);

        category.setRuleset(std::move(mOldRulesets.back()));
        mOldRulesets.pop_back();
    }

    tournament.changeCategories(categoryIds);
}

std::string ChangeCategoriesRulesetAction::getDescription() const {
    return "Change categories ruleset";
}

