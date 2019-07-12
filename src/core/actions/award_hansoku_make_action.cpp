#include "core/actions/award_hansoku_make_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

AwardHansokuMakeAction::AwardHansokuMakeAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(categoryId, matchId)
    , mPlayerIndex(playerIndex)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> AwardHansokuMakeAction::freshClone() const {
    return std::make_unique<AwardHansokuMakeAction>(mCategoryId, mMatchId, mPlayerIndex, mMasterTime);
}

std::string AwardHansokuMakeAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Award Hansoku-Make to White";
    return "Award Hansoku-Make to Blue";
}

void AwardHansokuMakeAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAddHansokuMake(match, mPlayerIndex))
        return;

    save(match);
    ruleset.addHansokuMake(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::HANSOKU_MAKE, mPlayerIndex, match.currentDuration(mMasterTime)});
    notify(tournament, match);
}

void AwardHansokuMakeAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

