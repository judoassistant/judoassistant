#include "core/actions/start_osaekomi_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

StartOsaekomiAction::StartOsaekomiAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(categoryId, matchId)
    , mMasterTime(masterTime)
    , mPlayerIndex(playerIndex)
{}

std::unique_ptr<Action> StartOsaekomiAction::freshClone() const {
    return std::make_unique<StartOsaekomiAction>(mCategoryId, mMatchId, mPlayerIndex, mMasterTime);
}

std::string StartOsaekomiAction::getDescription() const {
    return "Start Osaekomi";
}

void StartOsaekomiAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;

    auto &match = category.getMatch(mMatchId);
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

