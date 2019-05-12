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

    std::vector<CategoryId> categoryIds;
    for (auto categoryId : mCategoryIds) {
        if (tournament.containsCategory(categoryId))
            categoryIds.push_back(categoryId);
    }

    for (auto categoryId : categoryIds) {
        CategoryStore & category = tournament.getCategory(categoryId);

        mOldDrawSystems.push_back(category.setDrawSystem(drawSystem->clone()));
    }

    mDrawAction = std::make_unique<DrawCategoriesAction>(categoryIds, mSeed);
    mDrawAction->redo(tournament);

    tournament.changeCategories(categoryIds);
}

void ChangeCategoriesDrawSystemAction::undoImpl(TournamentStore & tournament) {
    std::vector<CategoryId> categoryIds;
    for (auto categoryId : mCategoryIds) {
        if (tournament.containsCategory(categoryId))
            categoryIds.push_back(categoryId);
    }

    assert(categoryIds.size() == mOldDrawSystems.size());

    mDrawAction->undo(tournament);
    mDrawAction.reset();

    for (auto i = categoryIds.rbegin(); i != categoryIds.rend(); ++i) {
        auto categoryId = *i;

        CategoryStore & category = tournament.getCategory(categoryId);

        category.setDrawSystem(std::move(mOldDrawSystems.back()));
        mOldDrawSystems.pop_back();
    }

    tournament.changeCategories(categoryIds);
}

std::string ChangeCategoriesDrawSystemAction::getDescription() const {
    return "Change categories draw system";
}

