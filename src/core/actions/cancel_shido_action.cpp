#include "core/actions/cancel_shido_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

CancelShidoAction::CancelShidoAction(CombinedId combinedId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(combinedId)
    , mPlayerIndex(playerIndex)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> CancelShidoAction::freshClone() const {
    return std::make_unique<CancelShidoAction>(mCombinedId, mPlayerIndex, mMasterTime);
}

std::string CancelShidoAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Cancel Shido to White";
    return "Cancel Shido to Blue";
}

void CancelShidoAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;
    auto &match = category.getMatch(mCombinedId.getMatchId());

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canCancelShido(match, mPlayerIndex))
        return;

    save(match);
    ruleset.cancelShido(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::CANCEL_SHIDO, mPlayerIndex, match.currentDuration(mMasterTime)});

    if (ruleset.shouldPause(match, mMasterTime))
        ruleset.pause(match, mMasterTime);

    notify(tournament, match);
}

void CancelShidoAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

