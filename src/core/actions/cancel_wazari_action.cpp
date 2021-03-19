#include "core/actions/cancel_wazari_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

CancelWazariAction::CancelWazariAction(CombinedId combinedId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(combinedId)
    , mPlayerIndex(playerIndex)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> CancelWazariAction::freshClone() const {
    return std::make_unique<CancelWazariAction>(mCombinedId, mPlayerIndex, mMasterTime);
}

std::string CancelWazariAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Cancel Wazari to White";
    else
        return "Cancel Wazari to Blue";
}

void CancelWazariAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;
    auto &match = category.getMatch(mCombinedId.getMatchId());

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canCancelWazari(match, mPlayerIndex))
        return;

    save(match);
    ruleset.cancelWazari(match, mPlayerIndex, mMasterTime);

    match.pushEvent({MatchEventType::CANCEL_WAZARI, mPlayerIndex, match.currentDuration(mMasterTime)});

    if (ruleset.shouldPause(match, mMasterTime))
        ruleset.pause(match, mMasterTime);

    notify(tournament, match);
}

void CancelWazariAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

