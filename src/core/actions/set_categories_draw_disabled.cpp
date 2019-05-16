#include "core/actions/set_categories_draw_disabled.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"

SetCategoriesDrawDisabled::SetCategoriesDrawDisabled(std::vector<CategoryId> categoryIds, bool disabled)
    : SetCategoriesDrawDisabled(categoryIds, disabled, getSeed())
{}

SetCategoriesDrawDisabled::SetCategoriesDrawDisabled(std::vector<CategoryId> categoryIds, bool disabled, unsigned int seed)
    : mCategoryIds(categoryIds)
    , mDisabled(disabled)
    , mSeed(seed)
{}

std::unique_ptr<Action> SetCategoriesDrawDisabled::freshClone() const {
    return std::make_unique<SetCategoriesDrawDisabled>(mCategoryIds, mDisabled, mSeed);
}

void SetCategoriesDrawDisabled::redoImpl(TournamentStore & tournament) {
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;
        const auto &category = tournament.getCategory(categoryId);
        if (category.isDrawDisabled() == mDisabled)
            continue;

        mChangedCategories.push_back(categoryId);
    }

    for (auto categoryId : mChangedCategories) {
        CategoryStore & category = tournament.getCategory(categoryId);
        category.setDrawDisabled(mDisabled);
    }

    mDrawAction = std::make_unique<DrawCategoriesAction>(mChangedCategories, mSeed);
    mDrawAction->redo(tournament);

    tournament.changeCategories(mChangedCategories);
}

void SetCategoriesDrawDisabled::undoImpl(TournamentStore & tournament) {
    for (auto i = mChangedCategories.rbegin(); i != mChangedCategories.rend(); ++i) {
        auto categoryId = *i;

        CategoryStore & category = tournament.getCategory(categoryId);
        category.setDrawDisabled(!mDisabled);
    }

    mDrawAction->undo(tournament);
    tournament.changeCategories(mChangedCategories);

    // clean-up
    mDrawAction.reset();
    mChangedCategories.clear();
}

std::string SetCategoriesDrawDisabled::getDescription() const {
    if (mDisabled)
        return "Disabled categories match drawing";
    return "Enable categories match drawing";
}

