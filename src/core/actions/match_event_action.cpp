#include "core/actions/match_event_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"

MatchEventAction::MatchEventAction(CategoryId categoryId, MatchId matchId)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
{}

void MatchEventAction::save(const MatchStore &match, unsigned int eventsToSave ) {
    mDidSave = true;
    mPrevStatus = match.getStatus();
    mPrevWhiteScore = match.getScore(MatchStore::PlayerIndex::WHITE);
    mPrevBlueScore = match.getScore(MatchStore::PlayerIndex::BLUE);
    mPrevGoldenScore = match.isGoldenScore();
    mPrevResumeTime = match.getResumeTime();
    mPrevDuration = match.getDuration();
    mPrevEventSize = match.getEvents().size();
    mPrevBye = match.isBye();
    mPrevOsaekomi = match.getOsaekomi();
    mPrevHasAwardedOsaekomiWazari = match.hasAwardedOsaekomiWazari();

    const auto &events = match.getEvents();
    eventsToSave = std::min(eventsToSave, static_cast<unsigned int>(events.size()));
    for (auto i = events.rbegin(); i != events.rbegin() + eventsToSave; ++i)
        mSavedEvents.push_back(*i);
}

void MatchEventAction::recover(TournamentStore &tournament) {
    assert(mDidSave);

    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    auto &category = tournament.getCategory(mCategoryId);
    auto &match = category.getMatch(mMatchId);
    auto updatedStatus = match.getStatus();

    match.setStatus(mPrevStatus);
    match.getScore(MatchStore::PlayerIndex::WHITE) = mPrevWhiteScore;
    match.getScore(MatchStore::PlayerIndex::BLUE) = mPrevBlueScore;

    match.setGoldenScore(mPrevGoldenScore);
    match.setResumeTime(mPrevResumeTime);
    match.setDuration(mPrevDuration);
    match.setBye(mPrevBye);

    match.setHasAwardedOsaekomiWazari(mPrevHasAwardedOsaekomiWazari);
    match.setOsaekomi(mPrevOsaekomi);

    if (match.getEvents().size() < mPrevEventSize) {
        assert(mPrevEventSize - match.getEvents().size() <= mSavedEvents.size());
        while (mPrevEventSize - match.getEvents().size() > mSavedEvents.size())
            match.popEvent(); // pop added events

        for (auto &event : mSavedEvents)
            match.pushEvent(event);
        mSavedEvents.clear();
    }
    else {
        // pop events if neccesary
        while (match.getEvents().size() > mPrevEventSize)
            match.popEvent();
    }

    // Updates tatami groups
    auto blockLocation = category.getLocation(match.getType());

    if (blockLocation && updatedStatus != mPrevStatus) {
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

        if (mPrevStatus == MatchStatus::NOT_STARTED)
            ++(categoryStatus.notStartedMatches);
        else if (mPrevStatus == MatchStatus::PAUSED || mPrevStatus == MatchStatus::UNPAUSED)
            ++(categoryStatus.startedMatches);
        else if (mPrevStatus == MatchStatus::FINISHED)
            ++(categoryStatus.finishedMatches);

        auto &concurrentGroup = tournament.getTatamis().at(blockLocation->sequentialGroup.concurrentGroup);
        concurrentGroup.updateStatus(match);

        std::pair<CategoryId, MatchType> block{category.getId(), match.getType()};
        tournament.changeTatamis({*blockLocation}, {block});
    }

    // Notify of match changed
    tournament.changeMatches(match.getCategory(), {match.getId()});

    // Notify results
    if (updatedStatus == MatchStatus::FINISHED || mPrevStatus == MatchStatus::FINISHED)
        tournament.resetCategoryResults({match.getCategory()});
}

bool MatchEventAction::shouldRecover() {
    return mDidSave;
}

void MatchEventAction::notify(TournamentStore &tournament, const MatchStore &match) {
    auto &category = tournament.getCategory(match.getCategory());

    // update category status
    auto & categoryStatus = category.getStatus(match.getType());
    if (mPrevStatus == MatchStatus::NOT_STARTED) {
        assert(categoryStatus.notStartedMatches > 0);
        --(categoryStatus.notStartedMatches);
    }
    else if (mPrevStatus == MatchStatus::PAUSED || mPrevStatus == MatchStatus::UNPAUSED) {
        assert(categoryStatus.startedMatches > 0);
        --(categoryStatus.startedMatches);
    }
    else if (mPrevStatus == MatchStatus::FINISHED) {
        assert(categoryStatus.finishedMatches > 0);
        --(categoryStatus.finishedMatches);
    }

    if (match.getStatus() == MatchStatus::NOT_STARTED)
        ++(categoryStatus.notStartedMatches);
    else if (match.getStatus() == MatchStatus::PAUSED || match.getStatus() == MatchStatus::UNPAUSED)
        ++(categoryStatus.startedMatches);
    else if (match.getStatus() == MatchStatus::FINISHED)
        ++(categoryStatus.finishedMatches);

    // Updates tatami groups
    auto blockLocation = category.getLocation(match.getType());
    if (blockLocation && match.getStatus() != mPrevStatus) {
        auto &concurrentGroup = tournament.getTatamis().at(blockLocation->sequentialGroup.concurrentGroup);
        concurrentGroup.updateStatus(match);

        std::pair<CategoryId, MatchType> block{category.getId(), match.getType()};
        tournament.changeTatamis({*blockLocation}, {block});
    }

    // Notify draw system
    // Changes to draws can only occur if the match was finished or is finished
    if (mPrevStatus == MatchStatus::FINISHED || match.getStatus() == MatchStatus::FINISHED) {
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

