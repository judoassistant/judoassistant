#include "core/actions/stop_osaekomi_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

StopOsaekomiAction::StopOsaekomiAction(CombinedId combinedId, std::chrono::milliseconds masterTime)
    : MatchEventAction(combinedId)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> StopOsaekomiAction::freshClone() const {
    return std::make_unique<StopOsaekomiAction>(mCombinedId, mMasterTime);
}

std::string StopOsaekomiAction::getDescription() const {
    return "Stop Osaekomi";
}

void StopOsaekomiAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;

    auto &match = category.getMatch(mCombinedId.getMatchId());
    const auto &ruleset = category.getRuleset();

    if (!ruleset.canStopOsaekomi(match, mMasterTime))
        return;

    save(match);
    ruleset.stopOsaekomi(match, mMasterTime);
    if (ruleset.shouldPause(match, mMasterTime))
        ruleset.pause(match, mMasterTime);

    notify(tournament, match);
}

void StopOsaekomiAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

