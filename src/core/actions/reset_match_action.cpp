#include "core/actions/reset_match_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

ResetMatchAction::ResetMatchAction(CategoryId categoryId, MatchId matchId)
    : mCategoryId(categoryId), mMatchId(matchId)
{}

std::unique_ptr<Action> ResetMatchAction::freshClone() const {
    return std::make_unique<ResetMatchAction>(mCategoryId, mMatchId);
}

std::string ResetMatchAction::getDescription() const {
    return "Reset match";
}

void ResetMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    mPrevState = std::move(match.getState());
    match.setState(MatchStore::State());

    auto updatedStatus = match.getStatus();
    auto prevStatus = mPrevState.status;

    // update category status
    auto & categoryStatus = category.getStatus(match.getType());
    if (updatedStatus != prevStatus) {
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
            auto &concurrentGroup = tournament.getTatamis().at(blockLocation->sequentialGroup.concurrentGroup);
            concurrentGroup.updateStatus(match);

            std::pair<CategoryId, MatchType> block{category.getId(), match.getType()};
            tournament.changeTatamis({*blockLocation}, {block});
        }
    }

    // Notify draw system
    // Changes to draws can only occur if the match was finished or is finished
    if (prevStatus == MatchStatus::FINISHED) {
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }

        tournament.resetCategoryResults({match.getCategory()});
    }

    // Notify of match changed
    tournament.changeMatches(match.getCategory(), {match.getId()});
}

void ResetMatchAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    auto updatedStatus = match.getStatus();
    auto prevStatus = mPrevState.status;
    match.setState(std::move(mPrevState));

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
            auto &concurrentGroup = tournament.getTatamis().at(blockLocation->sequentialGroup.concurrentGroup);
            concurrentGroup.updateStatus(match);

            std::pair<CategoryId, MatchType> block{category.getId(), match.getType()};

            tournament.changeTatamis({*blockLocation}, {block});
        }
    }


    // Notify of match changed
    tournament.changeMatches(match.getCategory(), {match.getId()});

    // Notify results
    if (prevStatus == MatchStatus::FINISHED)
        tournament.resetCategoryResults({match.getCategory()});
}

bool ResetMatchAction::shouldDisplay(CategoryId categoryId, MatchId matchId) const {
    return (mCategoryId == categoryId && mMatchId == matchId);
}

