#include "core/actions/pause_match_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

PauseMatchAction::PauseMatchAction(CombinedId combinedId, std::chrono::milliseconds masterTime)
    : MatchEventAction(combinedId)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> PauseMatchAction::freshClone() const {
    return std::make_unique<PauseMatchAction>(mCombinedId, mMasterTime);
}

std::string PauseMatchAction::getDescription() const {
    return "Pause match";
}

void PauseMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;
    auto &match = category.getMatch(mCombinedId.getMatchId());

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canPause(match, mMasterTime))
        return;

    save(match);
    ruleset.pause(match, mMasterTime);
    notify(tournament, match);
}

void PauseMatchAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}
