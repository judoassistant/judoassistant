#include "core/actions/award_shido_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

AwardShidoAction::AwardShidoAction(CombinedId combinedId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(combinedId)
    , mPlayerIndex(playerIndex)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> AwardShidoAction::freshClone() const {
    return std::make_unique<AwardShidoAction>(mCombinedId, mPlayerIndex, mMasterTime);
}

std::string AwardShidoAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Award Shido to White";
    return "Award Shido to Blue";
}

void AwardShidoAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;
    auto &match = category.getMatch(mCombinedId.getMatchId());

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAwardShido(match, mPlayerIndex))
        return;

    save(match);
    ruleset.awardShido(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::SHIDO, mPlayerIndex, match.currentDuration(mMasterTime)});

    if (ruleset.shouldPause(match, mMasterTime))
        ruleset.pause(match, mMasterTime);

    notify(tournament, match);
}

void AwardShidoAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

