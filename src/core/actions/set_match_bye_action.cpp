#include "core/actions/set_match_bye_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

SetMatchByeAction::SetMatchByeAction(CategoryId categoryId, MatchId matchId, bool bye)
    : MatchEventAction(categoryId, matchId)
    , mBye(bye)
{}

void SetMatchByeAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    if (match.isBye() == mBye)
        return;

    if (mBye && match.getStatus() != MatchStatus::NOT_STARTED)
        return;

    save(match);
    match.setStatus(mBye ? MatchStatus::FINISHED : MatchStatus::NOT_STARTED);
    match.setBye(mBye);
    notify(tournament, match);
}

void SetMatchByeAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

std::unique_ptr<Action> SetMatchByeAction::freshClone() const {
    return std::make_unique<SetMatchByeAction>(mCategoryId, mMatchId, mBye);
}

std::string SetMatchByeAction::getDescription() const {
    return "Set match bye status";
}

