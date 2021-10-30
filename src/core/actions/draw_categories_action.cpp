#include <set>

#include "core/actions/draw_categories_action.hpp"
#include "core/actions/add_match_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/id.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tatami/location.hpp"
#include "core/stores/tournament_store.hpp"

DrawCategoriesAction::DrawCategoriesAction(const std::vector<CategoryId> &categoryIds)
    : DrawCategoriesAction(categoryIds, getSeed())
{}

std::string DrawCategoriesAction::getDescription() const {
    return "Draw categories";
}

DrawCategoriesAction::DrawCategoriesAction(const std::vector<CategoryId> &categoryIds, unsigned int seed)
    : mCategoryIds(categoryIds)
    , mSeed(seed)
{}

std::unique_ptr<Action> DrawCategoriesAction::freshClone() const {
    return std::make_unique<DrawCategoriesAction>(mCategoryIds, mSeed);
}

void DrawCategoriesAction::redoImpl(TournamentStore & tournament) {
    std::unordered_set<BlockLocation> changedLocations;
    std::vector<std::pair<CategoryId, MatchType>> changedBlocks;

    // find existing categories
    std::vector<CategoryId> categoryIds;
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;
        const auto &category = tournament.getCategory(categoryId);
        // Skip category if draw is disabled the category has no matches already
        if (category.isDrawDisabled() && category.getMatches().empty())
            continue;
        categoryIds.push_back(categoryId);
    }

    tournament.beginResetMatches(categoryIds);

    // Update categories
    for (auto categoryId : categoryIds) {
        // Delete all existing matches
        CategoryStore & category = tournament.getCategory(categoryId);

        for (const std::unique_ptr<MatchStore> &match : category.getMatches()) {
            std::optional<PlayerId> whitePlayer = match->getPlayer(MatchStore::PlayerIndex::WHITE);
            if (whitePlayer && tournament.containsPlayer(*whitePlayer))
                tournament.getPlayer(*whitePlayer).eraseMatch(CombinedId(categoryId, match->getId()));

            std::optional<PlayerId> bluePlayer = match->getPlayer(MatchStore::PlayerIndex::BLUE);
            if (bluePlayer && tournament.containsPlayer(*bluePlayer))
                tournament.getPlayer(*bluePlayer).eraseMatch(CombinedId(categoryId, match->getId()));
        }

        mOldMatches.push_back(category.clearMatches());
        mOldDrawSystems.push_back(category.getDrawSystem().clone());

        std::array<CategoryStatus, 2> status;
        status[static_cast<size_t>(MatchType::ELIMINATION)] = category.getStatus(MatchType::ELIMINATION);
        status[static_cast<size_t>(MatchType::FINAL)] = category.getStatus(MatchType::FINAL);
        mOldStati.push_back(std::move(status));

        // TODO: Change mAction to have AddMatchActions instead
        std::vector<std::unique_ptr<Action>> actions;
        if (!category.isDrawDisabled()) {
            // Init the category using the draw system
            std::vector<PlayerId> playerIds(category.getPlayers().begin(), category.getPlayers().end());
            std::sort(playerIds.begin(), playerIds.end()); // Required to ensure consistent ordering
            std::vector<std::unique_ptr<AddMatchAction>> addMatchActions = category.getDrawSystem().initCategory(tournament, category, playerIds, mSeed);

            for (auto &actionPtr : addMatchActions) {
                actionPtr->redo(tournament);
                actions.push_back(std::move(actionPtr));
            }
        }

        mActions.push_back(std::move(actions));

        // Compute category status
        category.setStatus(MatchType::ELIMINATION, CategoryStatus());
        category.setStatus(MatchType::FINAL, CategoryStatus());

        for (const auto &matchPtr : category.getMatches()) {
            const auto &match = *matchPtr;
            auto &status = category.getStatus(match.getType());

            if (match.getStatus() == MatchStatus::FINISHED)
                ++(status.finishedMatches);
            else if (match.getStatus() == MatchStatus::NOT_STARTED)
                ++(status.notStartedMatches);
            else
                ++(status.startedMatches);
        }

        // Update tatami locations
        for (MatchType type : {MatchType::FINAL, MatchType::ELIMINATION}) {
            std::optional<BlockLocation> location = category.getLocation(type);
            if (location) {
                changedLocations.insert(*location);
                changedBlocks.push_back({categoryId, type});
            }
        }
    }

    tournament.endResetMatches(categoryIds);
    tournament.resetCategoryResults(categoryIds);

    if (!changedLocations.empty()) {
        std::vector<BlockLocation> locations(changedLocations.begin(), changedLocations.end());
        tournament.getTatamis().recomputeBlocks(tournament, locations);
        tournament.changeTatamis(locations, changedBlocks);
    }
}

void DrawCategoriesAction::undoImpl(TournamentStore & tournament) {
    std::unordered_set<BlockLocation> changedLocations;
    std::vector<std::pair<CategoryId, MatchType>> changedBlocks;

    // find existing categories
    std::vector<CategoryId> categoryIds;
    for (auto categoryId : mCategoryIds) {
        if (!tournament.containsCategory(categoryId))
            continue;
        const auto &category = tournament.getCategory(categoryId);
        // Skip category if draw is disabled the category has no matches already
        if (category.isDrawDisabled() && category.getMatches().empty())
            continue;
        categoryIds.push_back(categoryId);
    }

    tournament.beginResetMatches(categoryIds);

    // Last in, first out
    for (auto i = categoryIds.rbegin(); i != categoryIds.rend(); ++i) {
        CategoryId categoryId = *i;

        CategoryStore & category = tournament.getCategory(categoryId);

        while (!mActions.back().empty()) {
            mActions.back().back()->undo(tournament);
            mActions.back().pop_back();
        }
        mActions.pop_back();

        category.setDrawSystem(std::move(mOldDrawSystems.back()));
        mOldDrawSystems.pop_back();

        auto oldStatus = std::move(mOldStati.back());
        mOldStati.pop_back();
        category.setStatus(MatchType::ELIMINATION, oldStatus[static_cast<size_t>(MatchType::ELIMINATION)]);
        category.setStatus(MatchType::FINAL, oldStatus[static_cast<size_t>(MatchType::FINAL)]);

        for (std::unique_ptr<MatchStore> & match : mOldMatches.back()) {
            std::optional<PlayerId> whitePlayer = match->getPlayer(MatchStore::PlayerIndex::WHITE);
            if (whitePlayer && tournament.containsPlayer(*whitePlayer))
                tournament.getPlayer(*whitePlayer).addMatch(match->getCombinedId());

            std::optional<PlayerId> bluePlayer = match->getPlayer(MatchStore::PlayerIndex::BLUE);
            if (bluePlayer && tournament.containsPlayer(*bluePlayer))
                tournament.getPlayer(*bluePlayer).addMatch(match->getCombinedId());

            category.pushMatch(std::move(match));
        }

        mOldMatches.pop_back();

        for (MatchType type : {MatchType::FINAL, MatchType::ELIMINATION}) {
            std::optional<BlockLocation> location = category.getLocation(type);
            if (location) {
                changedLocations.insert(*location);
                changedBlocks.push_back({categoryId, type});
            }
        }
    }

    tournament.endResetMatches(categoryIds);
    tournament.resetCategoryResults(categoryIds);

    if (!changedLocations.empty()) {
        std::vector<BlockLocation> locations(changedLocations.begin(), changedLocations.end());
        tournament.getTatamis().recomputeBlocks(tournament, locations);
        tournament.changeTatamis(locations, changedBlocks);
    }

}
