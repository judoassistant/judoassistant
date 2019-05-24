#include "core/actions/change_categories_ruleset_action.hpp"
#include "core/actions/reset_matches_action.hpp"
#include "core/id.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"

ChangeCategoriesRulesetAction::ChangeCategoriesRulesetAction(const std::vector<CategoryId> &categoryIds, RulesetIdentifier ruleset)
    : mCategoryIds(categoryIds)
    , mRuleset(ruleset)
{}

std::unique_ptr<Action> ChangeCategoriesRulesetAction::freshClone() const {
    return std::make_unique<ChangeCategoriesRulesetAction>(mCategoryIds, mRuleset);
}

void ChangeCategoriesRulesetAction::redoImpl(TournamentStore & tournament) {
    auto ruleset = Ruleset::getRuleset(mRuleset);

    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;
        const CategoryStore & category = tournament.getCategory(categoryId);
        if (category.getRuleset().getIdentifier() == mRuleset)
            continue;
        mChangedCategories.push_back(categoryId);
    }

    for (auto categoryId : mChangedCategories) {
        CategoryStore & category = tournament.getCategory(categoryId);

        mOldRulesets.push_back(category.setRuleset(ruleset->clone()));
    }

    mResetAction = std::make_unique<ResetMatchesAction>(mChangedCategories);
    mResetAction->redo(tournament);

    tournament.changeCategories(mChangedCategories);
}

void ChangeCategoriesRulesetAction::undoImpl(TournamentStore & tournament) {
    mResetAction->undo(tournament);
    mResetAction.reset();

    for (auto i = mChangedCategories.rbegin(); i != mChangedCategories.rend(); ++i) {
        auto categoryId = *i;

        CategoryStore & category = tournament.getCategory(categoryId);

        category.setRuleset(std::move(mOldRulesets.back()));
        mOldRulesets.pop_back();
    }

    tournament.changeCategories(mChangedCategories);
    mChangedCategories.clear();
}

std::string ChangeCategoriesRulesetAction::getDescription() const {
    return "Change categories ruleset";
}

