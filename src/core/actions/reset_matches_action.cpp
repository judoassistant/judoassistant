#include "core/actions/reset_matches_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

ResetMatchesAction::ResetMatchesAction(CategoryId categoryId, const std::vector<MatchId> &matchIds)
    : mCategoryId(categoryId), mMatchIds(matchIds)
{}

std::unique_ptr<Action> ResetMatchesAction::freshClone() const {
    return std::make_unique<ResetMatchesAction>(mCategoryId, mMatchIds);
}

std::string ResetMatchesAction::getDescription() const {
    return "Reset match";
}

void ResetMatchesAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);

    std::unordered_set<BlockLocation> changedBlockLocations;
    std::unordered_set<std::pair<CategoryId, MatchType>> changedBlocks;
    bool shouldUpdateDraw = false;

    for (auto matchId : mMatchIds) {
        if (!category.containsMatch(matchId))
            continue;
        auto &match = category.getMatch(matchId);

        auto prevStatus = match.getState().status;

        mPrevStates.push(std::move(match.getState()));
        mPrevEvents.push(std::move(match.getEvents()));
        match.setState(MatchStore::State());
        match.clearEvents();

        auto updatedStatus = match.getStatus();

        // update category status
        if (updatedStatus != prevStatus) {
            auto & categoryStatus = category.getStatus(match.getType());
            if (prevStatus == MatchStatus::NOT_STARTED) {
                assert(categoryStatus.notStartedMatches > 0);
                --(categoryStatus.notStartedMatches);
            }
            else if (prevStatus == MatchStatus::PAUSED || prevStatus == MatchStatus::UNPAUSED) {
                assert(categoryStatus.startedMatches > 0);
                --(categoryStatus.startedMatches);
            }
            else if (prevStatus == MatchStatus::FINISHED) {
                assert(categoryStatus.finishedMatches > 0);
                --(categoryStatus.finishedMatches);
            }


            ++(categoryStatus.notStartedMatches);

            // Updates tatami groups
            auto blockLocation = category.getLocation(match.getType());
            if (blockLocation) {
                std::pair<CategoryId, MatchType> block{category.getId(), match.getType()};
                changedBlockLocations.insert(*blockLocation);
                changedBlocks.insert(block);
                auto &concurrentGroup = tournament.getTatamis().at(blockLocation->sequentialGroup.concurrentGroup);
                concurrentGroup.updateStatus(match);
            }

            if (prevStatus == MatchStatus::FINISHED)
                shouldUpdateDraw = true;
        }
    }

    if (!changedBlockLocations.empty()) {
        std::vector<BlockLocation> locations(changedBlockLocations.begin(), changedBlockLocations.end());
        std::vector<std::pair<CategoryId, MatchType>> blocks(changedBlocks.begin(), changedBlocks.end());

        tournament.changeTatamis(locations, blocks);
    }

    // Notify draw system
    // Changes to draws can only occur if the match was finished or is finished
    if (shouldUpdateDraw) {
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }

        tournament.resetCategoryResults({mCategoryId});
    }

    // Notify of match changed
    tournament.changeMatches(mCategoryId, mMatchIds);
}

void ResetMatchesAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);

    // undo draw actions
    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    // recover all matches
    std::unordered_set<BlockLocation> changedBlockLocations;
    std::unordered_set<std::pair<CategoryId, MatchType>> changedBlocks;
    bool shouldUpdateResults = false;

    for (auto it = mMatchIds.rbegin(); it != mMatchIds.rend(); ++it) {
        auto matchId = *it;

        if (!category.containsMatch(matchId))
            return;
        auto &match = category.getMatch(matchId);

        auto updatedStatus = match.getStatus();
        auto prevStatus = mPrevStates.top().status;
        match.setState(std::move(mPrevStates.top()));
        match.setEvents(std::move(mPrevEvents.top()));

        mPrevStates.pop();
        mPrevEvents.pop();

        // Update category and tatamis if matches went to/from finished or not_started
        if (updatedStatus != prevStatus) {
            auto &categoryStatus = category.getStatus(match.getType());

            assert(categoryStatus.notStartedMatches > 0);
            --(categoryStatus.notStartedMatches);

            if (prevStatus == MatchStatus::NOT_STARTED)
                ++(categoryStatus.notStartedMatches);
            else if (prevStatus == MatchStatus::PAUSED || prevStatus == MatchStatus::UNPAUSED)
                ++(categoryStatus.startedMatches);
            else if (prevStatus == MatchStatus::FINISHED)
                ++(categoryStatus.finishedMatches);

            auto blockLocation = category.getLocation(match.getType());
            if (blockLocation) {
                std::pair<CategoryId, MatchType> block{category.getId(), match.getType()};
                changedBlockLocations.insert(*blockLocation);
                changedBlocks.insert(block);
                auto &concurrentGroup = tournament.getTatamis().at(blockLocation->sequentialGroup.concurrentGroup);
                concurrentGroup.updateStatus(match);
            }

            if (prevStatus == MatchStatus::FINISHED)
                shouldUpdateResults = true;
        }
    }

    // Notify of match changed
    tournament.changeMatches(mCategoryId, mMatchIds);

    if (!changedBlockLocations.empty()) {
        std::vector<BlockLocation> locations(changedBlockLocations.begin(), changedBlockLocations.end());
        std::vector<std::pair<CategoryId, MatchType>> blocks(changedBlocks.begin(), changedBlocks.end());

        tournament.changeTatamis(locations, blocks);
    }

    // Notify results
    if (shouldUpdateResults)
        tournament.resetCategoryResults({mCategoryId});
}

bool ResetMatchesAction::shouldDisplay(CategoryId categoryId, MatchId matchId) const {
    if (mCategoryId != categoryId)
        return false;

    for (auto id : mMatchIds) {
        if (id == matchId)
            return true;
    }

    return false;
}

