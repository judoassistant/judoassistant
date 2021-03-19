#include "core/actions/resume_match_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

ResumeMatchAction::ResumeMatchAction(CombinedId combinedId, std::chrono::milliseconds masterTime)
    : MatchEventAction(combinedId)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> ResumeMatchAction::freshClone() const {
    return std::make_unique<ResumeMatchAction>(mCombinedId, mMasterTime);
}

std::string ResumeMatchAction::getDescription() const {
    return "Resume match";
}

void ResumeMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;

    auto &match = category.getMatch(mCombinedId.getMatchId());
    const auto &ruleset = category.getRuleset();

    if (!ruleset.canResume(match, mMasterTime))
        return;

    save(match);
    ruleset.resume(match, mMasterTime);
    notify(tournament, match);
}

void ResumeMatchAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}
