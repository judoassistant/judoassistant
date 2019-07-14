#include "core/actions/cancel_hansoku_make_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

CancelHansokuMakeAction::CancelHansokuMakeAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(categoryId, matchId)
    , mPlayerIndex(playerIndex)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> CancelHansokuMakeAction::freshClone() const {
    return std::make_unique<CancelHansokuMakeAction>(mCategoryId, mMatchId, mPlayerIndex, mMasterTime);
}

std::string CancelHansokuMakeAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Cancel Hansoku-Make to White";
    return "Cancel Hansoku-Make to Blue";
}

void CancelHansokuMakeAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canCancelHansokuMake(match, mPlayerIndex))
        return;

    save(match);
    ruleset.cancelHansokuMake(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::CANCEL_HANSOKU_MAKE, mPlayerIndex, match.currentDuration(mMasterTime)});
    notify(tournament, match);
}

void CancelHansokuMakeAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

