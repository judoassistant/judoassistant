#include "actions/match_actions.hpp"
#include "exception.hpp"
#include "stores/category_store.hpp"
#include "stores/tournament_store.hpp"
#include "rulesets/ruleset.hpp"
#include "draw_systems/draw_system.hpp"

AddMatchAction::AddMatchAction(TournamentStore & tournament, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId)
    : AddMatchAction(MatchId::generate(tournament.getCategory(categoryId)), categoryId, type, title, bye, whitePlayerId, bluePlayerId)
{}

AddMatchAction::AddMatchAction(MatchId id, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId)
    : mId(id)
    , mCategoryId(categoryId)
    , mType(type)
    , mTitle(title)
    , mBye(bye)
    , mWhitePlayerId(whitePlayerId)
    , mBluePlayerId(bluePlayerId)
{}

void AddMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);
    if (category.containsMatch(mId))
        throw ActionExecutionException("Failed to redo AddMatchAction. Match already exists.");

    std::optional<PlayerId> whitePlayerId;
    if (mWhitePlayerId && tournament.containsPlayer(*mWhitePlayerId)) {
        whitePlayerId = mWhitePlayerId;
        tournament.getPlayer(*whitePlayerId).addMatch(mCategoryId, mId);
    }

    std::optional<PlayerId> bluePlayerId;
    if (mBluePlayerId && tournament.containsPlayer(*mBluePlayerId)) {
        bluePlayerId = mBluePlayerId;
        tournament.getPlayer(*bluePlayerId).addMatch(mCategoryId, mId);
    }

    category.pushMatch(std::make_unique<MatchStore>(mId, mCategoryId, mType, mTitle, mBye, whitePlayerId, bluePlayerId));
}

void AddMatchAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    CategoryStore & category = tournament.getCategory(mCategoryId);

    if (mWhitePlayerId && tournament.containsPlayer(*mWhitePlayerId))
        tournament.getPlayer(*mWhitePlayerId).eraseMatch(mCategoryId, mId);

    if (mBluePlayerId && tournament.containsPlayer(*mBluePlayerId))
        tournament.getPlayer(*mBluePlayerId).eraseMatch(mCategoryId, mId);

    category.popMatch();
}

MatchId AddMatchAction::getMatchId() {
    return mId;
}

std::unique_ptr<Action> AddMatchAction::freshClone() const {
    return std::make_unique<AddMatchAction>(mId, mCategoryId, mType, mTitle, mBye, mWhitePlayerId, mBluePlayerId);
}

std::string AddMatchAction::getDescription() const {
    return "Add match";
}

ResumeMatchAction::ResumeMatchAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> ResumeMatchAction::freshClone() const {
    return std::make_unique<ResumeMatchAction>(mCategoryId, mMatchId, mMasterTime);
}

std::string ResumeMatchAction::getDescription() const {
    return "Resume match";
}

void ResumeMatchAction::redoImpl(TournamentStore & tournament) {
    mDidResume = false;

    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;

    auto &match = category.getMatch(mMatchId);
    const auto &ruleset = category.getRuleset();

    if (!ruleset.canResume(match, mMasterTime))
        return;

    mDidResume = true;
    mPrevStatus = match.getStatus();
    mPrevResumeTime = match.getResumeTime();

    ruleset.resume(match, mMasterTime);
    tournament.changeMatches(mCategoryId, {mMatchId});
}

void ResumeMatchAction::undoImpl(TournamentStore & tournament) {
    if (!mDidResume)
        return;
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);
    assert(match.getStatus() == MatchStatus::UNPAUSED);
    match.setStatus(mPrevStatus); // TODO: Perhaps have a more encapsulated way of storing state
    match.setResumeTime(mPrevResumeTime);
    tournament.changeMatches(mCategoryId, {mMatchId});
}

PauseMatchAction::PauseMatchAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> PauseMatchAction::freshClone() const {
    return std::make_unique<PauseMatchAction>(mCategoryId, mMatchId, mMasterTime);
}

std::string PauseMatchAction::getDescription() const {
    return "Pause match";
}

void PauseMatchAction::redoImpl(TournamentStore & tournament) {
    mDidPause = false;

    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canPause(match, mMasterTime))
        return;

    mDidPause = true;
    mPrevDuration = match.getDuration();

    ruleset.pause(match, mMasterTime);
    tournament.changeMatches(mCategoryId, {mMatchId});
}

void PauseMatchAction::undoImpl(TournamentStore & tournament) {
    if (!mDidPause)
        return;
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);
    assert(match.getStatus() == MatchStatus::PAUSED);
    match.setStatus(MatchStatus::UNPAUSED);
    match.setDuration(mPrevDuration);
    tournament.changeMatches(mCategoryId, {mMatchId});
}

