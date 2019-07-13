#include "core/actions/award_wazari_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

AwardWazariAction::AwardWazariAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime, bool osaekomi)
    : MatchEventAction(categoryId, matchId)
    , mPlayerIndex(playerIndex)
    , mOsaekomi(osaekomi)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> AwardWazariAction::freshClone() const {
    return std::make_unique<AwardWazariAction>(mCategoryId, mMatchId, mPlayerIndex, mMasterTime, mOsaekomi);
}

std::string AwardWazariAction::getDescription() const {
    std::string res;
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        res += "Award Wazari to White";
    else
        res += "Award Wazari to Blue";

    if (mOsaekomi)
        res += " (Osaekomi)";
    return res;
}

void AwardWazariAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    if (mOsaekomi && match.isOsaekomiWazari())
        return;

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAddWazari(match, mPlayerIndex))
        return;

    save(match);
    ruleset.addWazari(match, mPlayerIndex, mMasterTime);

    if (mOsaekomi)
        match.setOsaekomiWazari(true);

    auto type = (mOsaekomi ? MatchEventType::WAZARI_OSAEKOMI : MatchEventType::WAZARI);
    match.pushEvent({type, mPlayerIndex, match.currentDuration(mMasterTime)});
    notify(tournament, match);
}

void AwardWazariAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

