#include "core/actions/start_osaekomi_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

StartOsaekomiAction::StartOsaekomiAction(CombinedId combinedId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(combinedId)
    , mMasterTime(masterTime)
    , mPlayerIndex(playerIndex)
{}

std::unique_ptr<Action> StartOsaekomiAction::freshClone() const {
    return std::make_unique<StartOsaekomiAction>(mCombinedId, mPlayerIndex, mMasterTime);
}

std::string StartOsaekomiAction::getDescription() const {
    return "Start Osaekomi";
}

void StartOsaekomiAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCombinedId.getCategoryId()))
        return;
    auto &category = tournament.getCategory(mCombinedId.getCategoryId());
    if (!category.containsMatch(mCombinedId.getMatchId()))
        return;

    auto &match = category.getMatch(mCombinedId.getMatchId());
    const auto &ruleset = category.getRuleset();

    if (!ruleset.canStartOsaekomi(match, mPlayerIndex))
        return;

    save(match);
    ruleset.startOsaekomi(match, mPlayerIndex, mMasterTime);
    if (match.getStatus() != MatchStatus::UNPAUSED && (match.getDuration() < ruleset.getNormalTime() || match.isGoldenScore()))
        ruleset.resume(match, mMasterTime);

    notify(tournament, match);
}

void StartOsaekomiAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

