#include "core/actions/set_categories_matches_hidden_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"

SetCategoriesMatchesHiddenAction::SetCategoriesMatchesHiddenAction(std::vector<CategoryId> categoryIds, bool hidden)
    : SetCategoriesMatchesHiddenAction(categoryIds, hidden, getSeed())
{}

SetCategoriesMatchesHiddenAction::SetCategoriesMatchesHiddenAction(std::vector<CategoryId> categoryIds, bool hidden, unsigned int seed)
    : mCategoryIds(categoryIds)
    , mHidden(hidden)
    , mSeed(seed)
{}

std::unique_ptr<Action> SetCategoriesMatchesHiddenAction::freshClone() const {
    return std::make_unique<SetCategoriesMatchesHiddenAction>(mCategoryIds, mHidden, mSeed);
}

std::vector<CategoryId> SetCategoriesMatchesHiddenAction::getCategoriesThatChange(const TournamentStore &tournament) const {
    std::vector<CategoryId> result;
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;

        const auto &category = tournament.getCategory(categoryId);
        if (category.areMatchesHidden() == mHidden)
            continue;

        result.push_back(categoryId);
    }

    return result;
}

std::vector<BlockLocation> getLocationsThatChange(const std::vector<CategoryId> changedCategories, const TournamentStore &tournament) {
    std::unordered_set<BlockLocation> changedLocations;

    for (const auto categoryId : changedCategories) {
        const auto &category = tournament.getCategory(categoryId);

        for (MatchType type : { MatchType::FINAL, MatchType::ELIMINATION }) {
            std::optional<BlockLocation> location = category.getLocation(type);
            if (location)
                changedLocations.insert(*location);
        }
    }

    return std::vector(changedLocations.begin(), changedLocations.end());
}

std::vector<std::pair<CategoryId, MatchType>> getBlocksThatChange(const std::vector<CategoryId> changedCategories, const TournamentStore &tournament) {
    std::vector<std::pair<CategoryId, MatchType>> changedBlocks;
    for (const auto categoryId : changedCategories) {
        for (MatchType type : { MatchType::FINAL, MatchType::ELIMINATION }) {
            changedBlocks.emplace_back(categoryId, type);
        }
    }

    return changedBlocks;
}

// Signals that the categories and locations have have been modified.
void SetCategoriesMatchesHiddenAction::signalChanges(const std::vector<CategoryId> &changedCategories, TournamentStore &tournament) {
    if (changedCategories.empty())
        return;

    tournament.changeCategories(changedCategories);

    const auto changedLocations = getLocationsThatChange(changedCategories, tournament);
    const auto changedBlocks = getBlocksThatChange(changedCategories, tournament);
    if (!changedLocations.empty()) {
        tournament.getTatamis().recomputeBlocks(tournament, changedLocations);
        tournament.changeTatamis(changedLocations, changedBlocks);
    }
}

void SetCategoriesMatchesHiddenAction::redoImpl(TournamentStore & tournament) {
    mChangedCategories = getCategoriesThatChange(tournament);

    for (auto categoryId : mChangedCategories) {
        CategoryStore & category = tournament.getCategory(categoryId);
        category.setMatchesHidden(mHidden);
    }

    signalChanges(mChangedCategories, tournament);
}

void SetCategoriesMatchesHiddenAction::undoImpl(TournamentStore & tournament) {
    for (auto i = mChangedCategories.rbegin(); i != mChangedCategories.rend(); ++i) {
        auto categoryId = *i;

        CategoryStore & category = tournament.getCategory(categoryId);
        category.setMatchesHidden(!mHidden);
    }

    signalChanges(mChangedCategories, tournament);

    // Clean-up
    mChangedCategories.clear();
}

std::string SetCategoriesMatchesHiddenAction::getDescription() const {
    return mHidden
        ? "Hide category matches"
        : "Show category matches";
}

bool SetCategoriesMatchesHiddenAction::doesRequireConfirmation(const TournamentStore &tournament) const {
    if (mHidden)
        return false;

    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;
        const auto &category = tournament.getCategory(categoryId);
        if (category.areMatchesHidden() == mHidden)
            continue;

        if (category.isStarted())
            return true;
    }

    return false;
}

