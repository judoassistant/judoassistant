#include <set>

#include "core/actions/erase_categories_action.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/stores/tatami/location.hpp"
#include "core/stores/tournament_store.hpp"

EraseCategoriesAction::EraseCategoriesAction(const std::vector<CategoryId> &categoryIds)
    : mCategoryIds(categoryIds)
{}

std::unique_ptr<Action> EraseCategoriesAction::freshClone() const {
    return std::make_unique<EraseCategoriesAction>(mCategoryIds);
}

struct BlockLocationIndexComp {
    typedef std::tuple<CategoryId, MatchType, BlockLocation> Type;
    bool operator()(const Type &a, const Type &b) const {
        const auto &aLocation = std::get<2>(a);
        const auto &bLocation = std::get<2>(b);

        if (aLocation.sequentialGroup.concurrentGroup.tatami.handle.index != bLocation.sequentialGroup.concurrentGroup.tatami.handle.index)
            return aLocation.sequentialGroup.concurrentGroup.tatami.handle.index < bLocation.sequentialGroup.concurrentGroup.tatami.handle.index;
        if (aLocation.sequentialGroup.concurrentGroup.handle.index != bLocation.sequentialGroup.concurrentGroup.handle.index)
            return aLocation.sequentialGroup.concurrentGroup.handle.index < bLocation.sequentialGroup.concurrentGroup.handle.index;
        if (aLocation.sequentialGroup.handle.index != bLocation.sequentialGroup.handle.index)
            return aLocation.sequentialGroup.handle.index < bLocation.sequentialGroup.handle.index;
        return aLocation.pos < bLocation.pos;
    }
};

void EraseCategoriesAction::redoImpl(TournamentStore & tournament) {
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;

        mErasedCategoryIds.push_back(categoryId);
    }

    // Find all locations and keep them in increasing order
    std::set<std::tuple<CategoryId, MatchType, BlockLocation>, BlockLocationIndexComp> locations;

    tournament.beginEraseCategories(mErasedCategoryIds);
    const auto &tatamis = tournament.getTatamis();

    for (auto categoryId : mErasedCategoryIds) {
        const CategoryStore & category = tournament.getCategory(categoryId);

        for (auto type : {MatchType::ELIMINATION, MatchType::FINAL}) {
            if (category.getLocation(type)) {
                auto block = std::make_pair(categoryId, type);
                auto location = tatamis.refreshLocation(*category.getLocation(type), block);
                locations.insert({categoryId, type, location});
            }
        }
    }

    // move categories away from the tatami in increasing order
    for (const auto &tuple : locations) {
        auto block = std::make_pair(std::get<0>(tuple), std::get<1>(tuple));
        auto location = std::get<2>(tuple);
        tournament.getTatamis().moveBlock(tournament, block, location, std::nullopt);
    }

    // erase the categories
    for (auto categoryId : mErasedCategoryIds) {
        CategoryStore & category = tournament.getCategory(categoryId);

        for (auto playerId : category.getPlayers()) {
            PlayerStore & player = tournament.getPlayer(playerId);
            player.eraseCategory(categoryId);
        }

        for (const std::unique_ptr<MatchStore> &match : category.getMatches()) {
            auto whitePlayerId = match->getPlayer(MatchStore::PlayerIndex::WHITE);
            if (whitePlayerId)
                tournament.getPlayer(*whitePlayerId).eraseMatch(match->getCombinedId());

            auto bluePlayerId = match->getPlayer(MatchStore::PlayerIndex::BLUE);
            if (bluePlayerId)
                tournament.getPlayer(*bluePlayerId).eraseMatch(match->getCombinedId());
        }

        mCategories.push(tournament.eraseCategory(categoryId));
    }

    tournament.endEraseCategories(mErasedCategoryIds);

    for (auto tuple : locations) {
        mBlocks.push_back({std::get<0>(tuple), std::get<1>(tuple)});
        mLocations.push_back(std::get<2>(tuple));
    }

    if (!locations.empty())
        tournament.changeTatamis(mLocations, mBlocks);
}

void EraseCategoriesAction::undoImpl(TournamentStore & tournament) {
    tournament.beginAddCategories(mErasedCategoryIds);

    // Update players that belong to the category
    while (!mCategories.empty()) {
        std::unique_ptr<CategoryStore> category = std::move(mCategories.top());

        for (auto playerId : category->getPlayers()) {
            PlayerStore & player = tournament.getPlayer(playerId);
            player.addCategory(category->getId());
        }

        for (const std::unique_ptr<MatchStore> &match : category->getMatches()) {
            auto whitePlayerId = match->getPlayer(MatchStore::PlayerIndex::WHITE);
            if (whitePlayerId)
                tournament.getPlayer(*whitePlayerId).addMatch(match->getCombinedId());

            auto bluePlayerId = match->getPlayer(MatchStore::PlayerIndex::BLUE);
            if (bluePlayerId)
                tournament.getPlayer(*bluePlayerId).addMatch(match->getCombinedId());
        }

        tournament.addCategory(std::move(category));
        mCategories.pop();
    }

    // Update tatamis in non-decreasing order
    // TODO: Consider adding a feature to support making multiple moves at once
    for (size_t i = 0; i < mBlocks.size(); ++i)
        tournament.getTatamis().moveBlock(tournament, mBlocks[i], std::nullopt, mLocations[i]);

    tournament.endAddCategories(mErasedCategoryIds);
    if (!mLocations.empty())
        tournament.changeTatamis(mLocations, mBlocks);

    mErasedCategoryIds.clear();
    mBlocks.clear();
    mLocations.clear();
}

std::string EraseCategoriesAction::getDescription() const {
    return "Erase players from category";
}
