#include "core/actions/stop_osaekomi_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

StopOsaekomiAction::StopOsaekomiAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime)
    : MatchEventAction(categoryId, matchId)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> StopOsaekomiAction::freshClone() const {
    return std::make_unique<StopOsaekomiAction>(mCategoryId, mMatchId, mMasterTime);
}

std::string StopOsaekomiAction::getDescription() const {
    return "Stop Osaekomi";
}

void StopOsaekomiAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;

    auto &match = category.getMatch(mMatchId);
    const auto &ruleset = category.getRuleset();

    if (!ruleset.canStopOsaekomi(match, mMasterTime))
        return;

    save(match);
    ruleset.stopOsaekomi(match, mMasterTime);
    notify(tournament, match);
}

void StopOsaekomiAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

