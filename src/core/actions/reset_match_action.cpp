#include "core/actions/reset_match_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

ResetMatchAction::ResetMatchAction(CategoryId categoryId, MatchId matchId)
    : MatchEventAction(categoryId, matchId)
{}

std::unique_ptr<Action> ResetMatchAction::freshClone() const {
    return std::make_unique<ResetMatchAction>(mCategoryId, mMatchId);
}

std::string ResetMatchAction::getDescription() const {
    return "Reset match";
}

void ResetMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    save(match, match.getEvents().size()); // save all events as well
    match.clear();

    notify(tournament, match);
}

void ResetMatchAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

bool ResetMatchAction::shouldDisplay(CategoryId categoryId, MatchId matchId) const {
    return (mCategoryId == categoryId && mMatchId == matchId);
}

