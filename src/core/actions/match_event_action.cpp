#include "core/actions/match_event_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"

MatchEventAction::MatchEventAction(const CombinedId &combinedId)
    : mCombinedId(combinedId)
{}

void MatchEventAction::save(const MatchStore &match, unsigned int eventsToSave ) {
    mDidSave = true;
    mPrevState = match.getState();
    mPrevEventSize = match.getEvents().size();
}

void MatchEventAction::recover(TournamentStore &tournament) {
    assert(mDidSave);

    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    auto &match = category.getMatch(mCombinedId.getMatchId());
    auto updatedStatus = match.getStatus();
    auto prevStatus = mPrevState.status;

    match.setState(std::move(mPrevState));

    assert(match.getEvents().size() >= mPrevEventSize);
    while (match.getEvents().size() > mPrevEventSize)
        match.popEvent();

    // Update category and tatamis if matches went to/from finished or not_started
    if (updatedStatus != prevStatus && (prevStatus == MatchStatus::NOT_STARTED || prevStatus == MatchStatus::FINISHED || updatedStatus == MatchStatus::NOT_STARTED || updatedStatus == MatchStatus::FINISHED)) {
        auto &categoryStatus = category.getStatus(match.getType());

        if (updatedStatus == MatchStatus::NOT_STARTED) {
            assert(categoryStatus.notStartedMatches > 0);
            --(categoryStatus.notStartedMatches);
        }
        else if (updatedStatus == MatchStatus::PAUSED || updatedStatus == MatchStatus::UNPAUSED) {
            assert(categoryStatus.startedMatches > 0);
            --(categoryStatus.startedMatches);
        }
        else if (updatedStatus == MatchStatus::FINISHED) {
            assert(categoryStatus.finishedMatches > 0);
            --(categoryStatus.finishedMatches);
        }

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
    tournament.changeMatches(match.getCategoryId(), {match.getId()});

    // Notify results
    if (updatedStatus == MatchStatus::FINISHED || prevStatus == MatchStatus::FINISHED)
        tournament.resetCategoryResults({match.getCategoryId()});
}

bool MatchEventAction::shouldRecover() {
    return mDidSave;
}

void MatchEventAction::notify(TournamentStore &tournament, const MatchStore &match) {
    auto &category = tournament.getCategory(match.getCategoryId());
    auto updatedStatus = match.getStatus();
    auto prevStatus = mPrevState.status;

    // update category status
    auto & categoryStatus = category.getStatus(match.getType());
    if (updatedStatus != prevStatus && (prevStatus == MatchStatus::NOT_STARTED || prevStatus == MatchStatus::FINISHED || updatedStatus == MatchStatus::NOT_STARTED || updatedStatus == MatchStatus::FINISHED)) {
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

        if (updatedStatus == MatchStatus::NOT_STARTED)
            ++(categoryStatus.notStartedMatches);
        else if (updatedStatus == MatchStatus::PAUSED || updatedStatus == MatchStatus::UNPAUSED)
            ++(categoryStatus.startedMatches);
        else if (updatedStatus == MatchStatus::FINISHED)
            ++(categoryStatus.finishedMatches);

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
    if (prevStatus == MatchStatus::FINISHED || match.getStatus() == MatchStatus::FINISHED) {
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }

        tournament.resetCategoryResults({match.getCategoryId()});
    }

    // Notify of match changed
    tournament.changeMatches(match.getCategoryId(), {match.getId()});
}

