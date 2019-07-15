#include "core/actions/award_ippon_action.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/category_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

AwardIpponAction::AwardIpponAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime, bool osaekomi)
    : MatchEventAction(categoryId, matchId)
    , mPlayerIndex(playerIndex)
    , mOsaekomi(osaekomi)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> AwardIpponAction::freshClone() const {
    return std::make_unique<AwardIpponAction>(mCategoryId, mMatchId, mPlayerIndex, mMasterTime, mOsaekomi);
}

std::string AwardIpponAction::getDescription() const {
    std::string res;
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        res += "Award Ippon to White";
    else
        res += "Award Ippon to Blue";

    if (mOsaekomi)
        res += " (Osaekomi)";
    return res;
}

void AwardIpponAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAwardIppon(match, mPlayerIndex))
        return;

    save(match);


    if (match.isOsaekomiWazari() && ruleset.canCancelWazari(match, mPlayerIndex)) // Cancel the osaekomi wazari if possible
        ruleset.cancelWazari(match, mPlayerIndex, mMasterTime);

    ruleset.awardIppon(match, mPlayerIndex, mMasterTime);
    auto type = (mOsaekomi ? MatchEventType::IPPON_OSAEKOMI : MatchEventType::IPPON);
    match.pushEvent({type, mPlayerIndex, match.currentDuration(mMasterTime)});
    if (ruleset.shouldPause(match, mMasterTime))
        ruleset.pause(match, mMasterTime);
    notify(tournament, match);
}

void AwardIpponAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

