#include <map>

#include "core/actions/add_category_action.hpp"
#include "core/actions/add_players_action.hpp"
#include "core/actions/add_players_to_category_action.hpp"
#include "core/actions/add_players_with_categories_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"

std::unordered_set<std::string> AddPlayersWithCategoriesAction::getUniqueCategoryNames(const std::vector<std::optional<std::string>> &categoryNames) const {
    std::unordered_set<std::string> uniqueNames;

    for (const auto &name : categoryNames) {
        if (!name)
            continue;
        uniqueNames.insert(*name);
    }

    return uniqueNames;
}

std::unordered_map<std::string, CategoryId> AddPlayersWithCategoriesAction::createOrGetCategoryIds(const std::unordered_set<std::string> &categoryNames, const TournamentStore &tournament) {
    std::unordered_map<std::string, CategoryId> categoryIds;

    for (const auto &categoryName : categoryNames) {
        std::optional<CategoryId> categoryId = tournament.getCategoryByName(categoryName);

        if (!categoryId) {
            categoryId = CategoryId::generate(tournament);
            mNewCategories.emplace_back(*categoryId, categoryName);
        }

        categoryIds[categoryName] = *categoryId;
    }

    return categoryIds;
}

AddPlayersWithCategoriesAction::AddPlayersWithCategoriesAction(const TournamentStore &tournament, const std::vector<PlayerFields> &playerFields, const std::vector<std::optional<std::string>> &playerCategories)
    : mPlayerFields(playerFields)
    , mSeed(getSeed())
{
    const std::unordered_set<std::string> categoryNames = getUniqueCategoryNames(playerCategories);
    const std::unordered_map<std::string, CategoryId> categoryIds = createOrGetCategoryIds(categoryNames, tournament);

    for (size_t i = 0; i < playerFields.size(); ++i) {
        mPlayerIds.push_back(PlayerId::generate(tournament));

        std::optional<std::string> playerCategoryName = playerCategories[i];
        mPlayerCategories.push_back(playerCategoryName ? std::make_optional(categoryIds.at(*playerCategoryName)) : std::nullopt);
    }
}

AddPlayersWithCategoriesAction::AddPlayersWithCategoriesAction(const std::vector<PlayerId> &playerIds, const std::vector<PlayerFields> &playerFields, const std::vector<std::optional<CategoryId>> &playerCategories, const std::vector<std::pair<CategoryId, std::string>> &newCategories, unsigned int seed)
    : mPlayerIds(playerIds)
    , mPlayerFields(playerFields)
    , mPlayerCategories(playerCategories)
    , mNewCategories(newCategories)
    , mSeed(seed)
{}

void AddPlayersWithCategoriesAction::redoImpl(TournamentStore & tournament) {
    // Add players
    mAddPlayerAction = std::make_unique<AddPlayersAction>(mPlayerIds, mPlayerFields);
    mAddPlayerAction->redo(tournament);

    // Add new categories
    for (const auto &pair : mNewCategories) {
        const CategoryId categoryId = pair.first;
        const std::string categoryName = pair.second;
        const RulesetIdentifier ruleset = Ruleset::getDefaultRuleset()->getIdentifier();
        const DrawSystemIdentifier drawSystem = DrawSystemIdentifier::POOL; // Will be updated later when adding players

        auto addCategoryAction = std::make_unique<AddCategoryAction>(categoryId, categoryName, ruleset, drawSystem);
        addCategoryAction->redo(tournament);
        mAddCategoryActions.push(std::move(addCategoryAction));
    }

    // Create a map describing which players belong to each category
    std::map<CategoryId, std::vector<PlayerId>> categoryPlayers;
    for (size_t i = 0; i < mPlayerIds.size(); ++i) {
        const std::optional<CategoryId> categoryId = mPlayerCategories[i];
        const PlayerId playerId = mPlayerIds[i];

        if (categoryId)
            categoryPlayers[*categoryId].push_back(playerId);
    }

    // Add players to categories
    for (const auto &pair : categoryPlayers) {
        const CategoryId categoryId = pair.first;
        const std::vector<PlayerId> &categoryPlayers = pair.second;

        auto addPlayersToCategoryAction = std::make_unique<AddPlayersToCategoryAction>(categoryId, categoryPlayers, mSeed);
        addPlayersToCategoryAction->redo(tournament);
        mAddPlayersToCategoryActions.push(std::move(addPlayersToCategoryAction));
    }
}

void AddPlayersWithCategoriesAction::undoImpl(TournamentStore & tournament) {
    while (!mAddPlayersToCategoryActions.empty()) {
        mAddPlayersToCategoryActions.top()->undo(tournament);
        mAddPlayersToCategoryActions.pop();
    }

    while (!mAddCategoryActions.empty()) {
        mAddCategoryActions.top()->undo(tournament);
        mAddCategoryActions.pop();
    }

    mAddPlayerAction->undo(tournament);
    mAddPlayerAction.reset();
}

std::unique_ptr<Action> AddPlayersWithCategoriesAction::freshClone() const {
    return std::make_unique<AddPlayersWithCategoriesAction>(mPlayerIds, mPlayerFields, mPlayerCategories, mNewCategories, mSeed);
}

std::string AddPlayersWithCategoriesAction::getDescription() const {
    return "Add players to categories";
}

