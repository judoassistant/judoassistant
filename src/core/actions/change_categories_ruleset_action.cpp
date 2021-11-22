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

std::vector<CategoryId> ChangeCategoriesRulesetAction::getCategoriesThatChange(const TournamentStore &tournament) const {
    std::vector<CategoryId> categoryIds;
    auto ruleset = Ruleset::getRuleset(mRuleset);

    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;

        const CategoryStore & category = tournament.getCategory(categoryId);
        if (category.getRuleset().getIdentifier() == mRuleset)
            continue;

        categoryIds.push_back(categoryId);
    }

    return categoryIds;
}

void ChangeCategoriesRulesetAction::redoImpl(TournamentStore & tournament) {
    std::unordered_set<BlockLocation> changedLocations;
    std::vector<std::pair<CategoryId, MatchType>> changedBlocks;


    mChangedCategories = getCategoriesThatChange(tournament);

    for (auto categoryId : mChangedCategories) {
        auto &category = tournament.getCategory(categoryId);

        if (category.getLocation(MatchType::ELIMINATION)) {
            changedLocations.insert(*category.getLocation(MatchType::ELIMINATION));
            changedBlocks.emplace_back(categoryId, MatchType::ELIMINATION);
        }

        if (category.getLocation(MatchType::FINAL)) {
            changedLocations.insert(*category.getLocation(MatchType::FINAL));
            changedBlocks.emplace_back(categoryId, MatchType::FINAL);
        }
    }

    auto ruleset = Ruleset::getRuleset(mRuleset);
    for (auto categoryId : mChangedCategories) {
        CategoryStore & category = tournament.getCategory(categoryId);

        mOldRulesets.push_back(category.setRuleset(ruleset->clone()));
    }

    mResetAction = std::make_unique<ResetMatchesAction>(mChangedCategories);
    mResetAction->redo(tournament);

    tournament.changeCategories(mChangedCategories);

    if (!changedLocations.empty()) {
        std::vector<BlockLocation> locations(changedLocations.begin(), changedLocations.end());
        tournament.changeTatamis(locations, changedBlocks);
    }
}

void ChangeCategoriesRulesetAction::undoImpl(TournamentStore & tournament) {
    std::unordered_set<BlockLocation> changedLocations;
    std::vector<std::pair<CategoryId, MatchType>> changedBlocks;

    mResetAction->undo(tournament);
    mResetAction.reset();

    for (auto i = mChangedCategories.rbegin(); i != mChangedCategories.rend(); ++i) {
        auto categoryId = *i;

        CategoryStore & category = tournament.getCategory(categoryId);

        category.setRuleset(std::move(mOldRulesets.back()));
        mOldRulesets.pop_back();

        if (category.getLocation(MatchType::ELIMINATION)) {
            changedLocations.insert(*category.getLocation(MatchType::ELIMINATION));
            changedBlocks.emplace_back(categoryId, MatchType::ELIMINATION);
        }

        if (category.getLocation(MatchType::FINAL)) {
            changedLocations.insert(*category.getLocation(MatchType::FINAL));
            changedBlocks.emplace_back(categoryId, MatchType::FINAL);
        }
    }

    tournament.changeCategories(mChangedCategories);
    mChangedCategories.clear();

    if (!changedLocations.empty()) {
        std::vector<BlockLocation> locations(changedLocations.begin(), changedLocations.end());
        tournament.changeTatamis(locations, changedBlocks);
    }
}

std::string ChangeCategoriesRulesetAction::getDescription() const {
    return "Change categories ruleset";
}

bool ChangeCategoriesRulesetAction::doesRequireConfirmation(const TournamentStore &tournament) const {
    for (const CategoryId &categoryId : getCategoriesThatChange(tournament)) {
        const auto &category = tournament.getCategory(categoryId);

        if (category.isStarted())
            return true;
    }

    return false;
}

