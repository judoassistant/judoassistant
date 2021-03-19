#include "core/actions/cancel_hansoku_make_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

CancelHansokuMakeAction::CancelHansokuMakeAction(CombinedId combinedId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(combinedId)
    , mPlayerIndex(playerIndex)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> CancelHansokuMakeAction::freshClone() const {
    return std::make_unique<CancelHansokuMakeAction>(mCombinedId, mPlayerIndex, mMasterTime);
}

std::string CancelHansokuMakeAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Cancel Hansoku-Make to White";
    return "Cancel Hansoku-Make to Blue";
}

void CancelHansokuMakeAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;
    auto &match = category.getMatch(mCombinedId.getMatchId());

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canCancelHansokuMake(match, mPlayerIndex))
        return;

    save(match);
    ruleset.cancelHansokuMake(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::CANCEL_HANSOKU_MAKE, mPlayerIndex, match.currentDuration(mMasterTime)});

    if (ruleset.shouldPause(match, mMasterTime))
        ruleset.pause(match, mMasterTime);

    notify(tournament, match);
}

void CancelHansokuMakeAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

