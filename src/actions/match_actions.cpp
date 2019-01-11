#include "actions/match_actions.hpp"
#include "exception.hpp"
#include "stores/category_store.hpp"
#include "stores/tournament_store.hpp"
#include "rulesets/ruleset.hpp"
#include "draw_systems/draw_system.hpp"

AddMatchAction::AddMatchAction(const TournamentStore & tournament, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId)
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
    : MatchEventAction(categoryId, matchId)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> ResumeMatchAction::freshClone() const {
    return std::make_unique<ResumeMatchAction>(mCategoryId, mMatchId, mMasterTime);
}

std::string ResumeMatchAction::getDescription() const {
    return "Resume match";
}

void ResumeMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;

    auto &match = category.getMatch(mMatchId);
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

PauseMatchAction::PauseMatchAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime)
    : MatchEventAction(categoryId, matchId)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> PauseMatchAction::freshClone() const {
    return std::make_unique<PauseMatchAction>(mCategoryId, mMatchId, mMasterTime);
}

std::string PauseMatchAction::getDescription() const {
    return "Pause match";
}

void PauseMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canPause(match, mMasterTime))
        return;

    save(match);
    ruleset.pause(match, mMasterTime);
    notify(tournament, match);
}

void PauseMatchAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

AwardIpponAction::AwardIpponAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(categoryId, matchId)
    , mPlayerIndex(playerIndex)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> AwardIpponAction::freshClone() const {
    return std::make_unique<AwardIpponAction>(mCategoryId, mMatchId, mPlayerIndex, mMasterTime);
}

std::string AwardIpponAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Award Ippon to White";
    return "Award Ippon to Blue";
}

void AwardIpponAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAddIppon(match, mPlayerIndex))
        return;

    save(match);
    ruleset.addIppon(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::IPPON, mPlayerIndex, match.currentDuration(mMasterTime)});
    notify(tournament, match);
}

void AwardIpponAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

AwardWazariAction::AwardWazariAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(categoryId, matchId)
    , mPlayerIndex(playerIndex)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> AwardWazariAction::freshClone() const {
    return std::make_unique<AwardWazariAction>(mCategoryId, mMatchId, mPlayerIndex, mMasterTime);
}

std::string AwardWazariAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Award Wazari to White";
    return "Award Wazari to Blue";
}

void AwardWazariAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAddWazari(match, mPlayerIndex))
        return;

    save(match);
    ruleset.addWazari(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::WAZARI, mPlayerIndex, match.currentDuration(mMasterTime)});
    notify(tournament, match);
}

void AwardWazariAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

AwardShidoAction::AwardShidoAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : MatchEventAction(categoryId, matchId)
    , mPlayerIndex(playerIndex)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> AwardShidoAction::freshClone() const {
    return std::make_unique<AwardShidoAction>(mCategoryId, mMatchId, mPlayerIndex, mMasterTime);
}

std::string AwardShidoAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Award Shido to White";
    return "Award Shido to Blue";
}

void AwardShidoAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAddShido(match, mPlayerIndex))
        return;

    save(match);
    ruleset.addShido(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::SHIDO, mPlayerIndex, match.currentDuration(mMasterTime)});
    notify(tournament, match);
}

void AwardShidoAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

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

bool AwardIpponAction::shouldDisplay(CategoryId categoryId, MatchId matchId) const {
    return (mCategoryId == categoryId && mMatchId == matchId);
}

bool AwardWazariAction::shouldDisplay(CategoryId categoryId, MatchId matchId) const {
    return (mCategoryId == categoryId && mMatchId == matchId);
}

bool AwardShidoAction::shouldDisplay(CategoryId categoryId, MatchId matchId) const {
    return (mCategoryId == categoryId && mMatchId == matchId);
}

bool AwardHansokuMakeAction::shouldDisplay(CategoryId categoryId, MatchId matchId) const {
    return (mCategoryId == categoryId && mMatchId == matchId);
}

MatchEventAction::MatchEventAction(CategoryId categoryId, MatchId matchId)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
{}

void MatchEventAction::save(const MatchStore &match) {
    mDidSave = true;
    mPrevStatus = match.getStatus();
    mPrevWhiteScore = match.getScore(MatchStore::PlayerIndex::WHITE);
    mPrevBlueScore = match.getScore(MatchStore::PlayerIndex::BLUE);
    mPrevGoldenScore = match.isGoldenScore();
    mPrevResumeTime = match.getResumeTime();
    mPrevDuration = match.getDuration();
    mPrevEventSize = match.getEvents().size();
}

void MatchEventAction::recover(TournamentStore &tournament) {
    assert(mDidSave);

    auto &category = tournament.getCategory(mCategoryId);
    auto &match = category.getMatch(mMatchId);
    auto updatedStatus = match.getStatus();

    match.setStatus(mPrevStatus);
    match.getScore(MatchStore::PlayerIndex::WHITE) = mPrevWhiteScore;
    match.getScore(MatchStore::PlayerIndex::BLUE) = mPrevBlueScore;

    match.setGoldenScore(mPrevGoldenScore);
    match.setResumeTime(mPrevResumeTime);
    match.setDuration(mPrevDuration);

    assert(match.getEvents().size() >= mPrevEventSize);
    while (match.getEvents().size() > mPrevEventSize)
        match.popEvent();

    // Updates tatami groups
    auto blockLocation = category.getLocation(match.getType());
    if (blockLocation && updatedStatus != mPrevStatus) {
        auto &concurrentGroup = tournament.getTatamis().at(blockLocation->sequentialGroup.concurrentGroup);
        concurrentGroup.updateStatus(match);
    }

    // Notify of match changed
    tournament.changeMatches(match.getCategory(), {match.getId()});

    // Notify draw system
    // Changes to draws can only occur if the match was finished or is finished
    if (mPrevStatus == MatchStatus::FINISHED || updatedStatus == MatchStatus::FINISHED) {
         const auto &drawSystem = category.getDrawSystem();
         auto drawActions = drawSystem.updateCategory(tournament, category);
         for (std::unique_ptr<Action> &action : drawActions) {
             action->redo(tournament);
             mDrawActions.push(std::move(action));
         }
    }
}

bool MatchEventAction::shouldRecover() {
    return mDidSave;
}

void MatchEventAction::notify(TournamentStore &tournament, const MatchStore &match) {
    auto &category = tournament.getCategory(match.getCategory());

    // Updates tatami groups
    auto blockLocation = category.getLocation(match.getType());
    if (blockLocation && match.getStatus() != mPrevStatus) {
        auto &concurrentGroup = tournament.getTatamis().at(blockLocation->sequentialGroup.concurrentGroup);
        concurrentGroup.updateStatus(match);
    }

    // Notify of match changed
    tournament.changeMatches(match.getCategory(), {match.getId()});

    // Notify draw system
    // Changes to draws can only occur if the match was finished or is finished
    if (mPrevStatus == MatchStatus::FINISHED || match.getStatus() == MatchStatus::FINISHED) {
         const auto &drawSystem = category.getDrawSystem();
         auto drawActions = drawSystem.updateCategory(tournament, category);
         for (std::unique_ptr<Action> &action : drawActions) {
             action->redo(tournament);
             mDrawActions.push(std::move(action));
         }
    }
}

