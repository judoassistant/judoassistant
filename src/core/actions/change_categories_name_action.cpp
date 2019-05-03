#include "core/actions/change_categories_name_action.hpp"
#include "core/id.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"

ChangeCategoriesNameAction::ChangeCategoriesNameAction(std::vector<CategoryId> categoryIds, const std::string &value)
    : mCategoryIds(categoryIds)
    , mValue(value)
{}

std::unique_ptr<Action> ChangeCategoriesNameAction::freshClone() const {
    return std::make_unique<ChangeCategoriesNameAction>(mCategoryIds, mValue);
}

void ChangeCategoriesNameAction::redoImpl(TournamentStore & tournament) {
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;

        CategoryStore & category = tournament.getCategory(categoryId);
        mOldValues.push_back(category.getName());
        category.setName(mValue);
    }
    tournament.changeCategories(mCategoryIds);
}

void ChangeCategoriesNameAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;
        assert(i != mOldValues.end());

        CategoryStore & category = tournament.getCategory(categoryId);

        category.setName(*i);

        std::advance(i, 1);
    }

    tournament.changeCategories(mCategoryIds);
    mOldValues.clear();
}

std::string ChangeCategoriesNameAction::getDescription() const {
    return "Change categories name";
}
