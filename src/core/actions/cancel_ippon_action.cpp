#include "core/actions/cancel_ippon_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

CancelIpponAction::CancelIpponAction(CombinedId combinedId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(combinedId)
    , mPlayerIndex(playerIndex)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> CancelIpponAction::freshClone() const {
    return std::make_unique<CancelIpponAction>(mCombinedId, mPlayerIndex, mMasterTime);
}

std::string CancelIpponAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Cancel Ippon to White";
    else
        return "Cancel Ippon to Blue";
}

void CancelIpponAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;
    auto &match = category.getMatch(mCombinedId.getMatchId());

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canCancelIppon(match, mPlayerIndex))
        return;

    save(match);
    ruleset.cancelIppon(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::CANCEL_IPPON, mPlayerIndex, match.currentDuration(mMasterTime)});

    if (ruleset.shouldPause(match, mMasterTime))
        ruleset.pause(match, mMasterTime);

    notify(tournament, match);
}

void CancelIpponAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

