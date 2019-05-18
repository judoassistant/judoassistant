#include "core/actions/reset_matches_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

ResetMatchesAction::ResetMatchesAction(const std::vector<CategoryId> &categoryIds)
    : mCategoryIds(categoryIds)
    , mMatchId(std::nullopt)
{}

ResetMatchesAction::ResetMatchesAction(CategoryId categoryId, MatchId matchId)
    : mCategoryIds({categoryId})
    , mMatchId(matchId)
{}

ResetMatchesAction::ResetMatchesAction(const std::vector<CategoryId> &categoryIds, std::optional<MatchId> matchId)
    : mCategoryIds(categoryIds)
    , mMatchId(matchId)
{
    assert((!mMatchId.has_value()) || mCategoryIds.size() == 1);
}

std::unique_ptr<Action> ResetMatchesAction::freshClone() const {
    return std::make_unique<ResetMatchesAction>(mCategoryIds, mMatchId);
}

std::string ResetMatchesAction::getDescription() const {
    return "Reset matches";
}

void ResetMatchesAction::redoImpl(TournamentStore & tournament) {
    std::unordered_set<BlockLocation> changedBlockLocations;
    std::unordered_set<std::pair<CategoryId, MatchType>> changedBlocks;
    std::unordered_set<CategoryId> categoryDrawUpdates;

    if (mMatchId.has_value()) {
        assert(mCategoryIds.size() == 1);

        const auto &category = tournament.getCategory(mCategoryIds.front());

        if (!category.containsMatch(*mMatchId))
            return;
        auto &match = category.getMatch(*mMatchId);

        if (match.getStatus() == MatchStatus::NOT_STARTED)
            return;

        mChangedMatches.push_back(match.getCombinedId());
    }
    else {
        for (auto categoryId : mCategoryIds) {
            if (!tournament.containsCategory(categoryId))
                continue;
            const auto &category = tournament.getCategory(categoryId);
            for (const auto &match : category.getMatches()) {
                if (match->getStatus() == MatchStatus::NOT_STARTED)
                    continue;

                mChangedMatches.push_back(match->getCombinedId());
            }
        }
    }

    if (mChangedMatches.empty())
        return;

    for (auto combinedId : mChangedMatches) {
        auto &category = tournament.getCategory(combinedId.first);
        auto &match = category.getMatch(combinedId.second);

        auto prevStatus = match.getState().status;

        mPrevStates.push(std::move(match.getState()));
        mPrevEvents.push(std::move(match.getEvents()));
        match.clearState();
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
                categoryDrawUpdates.insert(category.getId());
        }
    }

    if (!changedBlockLocations.empty()) {
        std::vector<BlockLocation> locations(changedBlockLocations.begin(), changedBlockLocations.end());
        std::vector<std::pair<CategoryId, MatchType>> blocks(changedBlocks.begin(), changedBlocks.end());

        tournament.changeTatamis(locations, blocks);
    }

    // Notify draw system
    // Changes to draws can only occur if the match was finished or is finished
    for (auto categoryId : categoryDrawUpdates) {
        auto &category = tournament.getCategory(categoryId);
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }
    }

    if (!categoryDrawUpdates.empty())
        tournament.resetCategoryResults(std::vector(categoryDrawUpdates.begin(), categoryDrawUpdates.end()));

    // Notify of match changed
    // TODO tournament.changeMatches(mCategoryId, mChangedMatches);
}

void ResetMatchesAction::undoImpl(TournamentStore & tournament) {
    if (mChangedMatches.empty())
        return;

    // undo draw actions
    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    // recover all matches
    std::unordered_set<BlockLocation> changedBlockLocations;
    std::unordered_set<std::pair<CategoryId, MatchType>> changedBlocks;
    std::unordered_set<CategoryId> categoryDrawUpdates;

    for (auto it = mChangedMatches.rbegin(); it != mChangedMatches.rend(); ++it) {
        auto combinedId = *it;
        auto &category = tournament.getCategory(combinedId.first);
        auto &match = category.getMatch(combinedId.second);

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
                categoryDrawUpdates.insert(category.getId());
        }
    }

    // Notify of match changed
    // TODO tournament.changeMatches(mCategoryId, mChangedMatches);
    mChangedMatches.clear();

    if (!changedBlockLocations.empty()) {
        std::vector<BlockLocation> locations(changedBlockLocations.begin(), changedBlockLocations.end());
        std::vector<std::pair<CategoryId, MatchType>> blocks(changedBlocks.begin(), changedBlocks.end());

        tournament.changeTatamis(locations, blocks);
    }

    // Notify results
    if (!categoryDrawUpdates.empty())
        tournament.resetCategoryResults(std::vector(categoryDrawUpdates.begin(), categoryDrawUpdates.end()));
}

bool ResetMatchesAction::shouldDisplay(CategoryId categoryId, MatchId matchId) const {
    bool res = false;
    for (auto el : mCategoryIds) {
        if (el == categoryId) {
            res = true;
            break;
        }
    }

    if (mMatchId.has_value())
        res &= (mMatchId == matchId);

    return res;
}

