#include "core/actions/pause_match_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

PauseMatchAction::PauseMatchAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime)
    : MatchEventAction(categoryId, matchId, masterTime)
{}

std::unique_ptr<Action> PauseMatchAction::freshClone() const {
    return std::make_unique<PauseMatchAction>(mCategoryId, mMatchId, mMasterTime);
}

std::string PauseMatchAction::getDescription() const {
    return "Pause match";
}

void PauseMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

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
