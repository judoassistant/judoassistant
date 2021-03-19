#include "core/actions/set_match_bye_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

SetMatchByeAction::SetMatchByeAction(CombinedId combinedId, bool bye)
    : MatchEventAction(combinedId)
    , mBye(bye)
{}

void SetMatchByeAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;
    auto &match = category.getMatch(mCombinedId.getMatchId());

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
    return std::make_unique<SetMatchByeAction>(mCombinedId, mBye);
}

std::string SetMatchByeAction::getDescription() const {
    return "Set match bye status";
}

