#include "core/actions/change_categories_draw_system_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"

ChangeCategoriesDrawSystemAction::ChangeCategoriesDrawSystemAction(std::vector<CategoryId> categoryIds, DrawSystemIdentifier drawSystem)
    : ChangeCategoriesDrawSystemAction(categoryIds, drawSystem, getSeed())
{}

ChangeCategoriesDrawSystemAction::ChangeCategoriesDrawSystemAction(std::vector<CategoryId> categoryIds, DrawSystemIdentifier drawSystem, unsigned int seed)
    : mCategoryIds(categoryIds)
    , mDrawSystem(drawSystem)
    , mSeed(seed)
{}

std::unique_ptr<Action> ChangeCategoriesDrawSystemAction::freshClone() const {
    return std::make_unique<ChangeCategoriesDrawSystemAction>(mCategoryIds, mDrawSystem, mSeed);
}

void ChangeCategoriesDrawSystemAction::redoImpl(TournamentStore & tournament) {
    auto drawSystem = DrawSystem::getDrawSystem(mDrawSystem);

    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;

        const CategoryStore & category = tournament.getCategory(categoryId);
        if (category.getDrawSystem().getIdentifier() == mDrawSystem)
            continue;
        mChangedCategories.push_back(categoryId);
    }

    for (auto categoryId : mChangedCategories) {
        CategoryStore & category = tournament.getCategory(categoryId);

        mOldDrawSystems.push_back(category.setDrawSystem(drawSystem->clone()));
    }

    mDrawAction = std::make_unique<DrawCategoriesAction>(mChangedCategories, mSeed);
    mDrawAction->redo(tournament);

    tournament.changeCategories(mChangedCategories);
}

void ChangeCategoriesDrawSystemAction::undoImpl(TournamentStore & tournament) {
    mDrawAction->undo(tournament);
    mDrawAction.reset();

    for (auto i = mChangedCategories.rbegin(); i != mChangedCategories.rend(); ++i) {
        auto categoryId = *i;

        CategoryStore & category = tournament.getCategory(categoryId);

        category.setDrawSystem(std::move(mOldDrawSystems.back()));
        mOldDrawSystems.pop_back();
    }

    tournament.changeCategories(mChangedCategories);
    mChangedCategories.clear();
}

std::string ChangeCategoriesDrawSystemAction::getDescription() const {
    return "Change categories draw system";
}

