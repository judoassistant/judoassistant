#include "core/actions/match_actions.hpp"
#include "core/exception.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/log.hpp"

// AddMatchAction::AddMatchAction(const TournamentStore & tournament, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId)
//     : AddMatchAction(MatchId::generate(tournament.getCategory(categoryId)), categoryId, type, title, bye, whitePlayerId, bluePlayerId)
// {}

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
    if (!ruleset.canAddIppon(match, mPlayerIndex))
        return;

    save(match);
    ruleset.addIppon(match, mPlayerIndex, mMasterTime);
    auto type = (mOsaekomi ? MatchEventType::IPPON_OSAEKOMI : MatchEventType::IPPON);
    match.pushEvent({type, mPlayerIndex, match.currentDuration(mMasterTime)});
    notify(tournament, match);
}

void AwardIpponAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

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

    if (mOsaekomi && match.hasAwardedOsaekomiWazari())
        return;

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAddWazari(match, mPlayerIndex))
        return;

    save(match);
    ruleset.addWazari(match, mPlayerIndex, mMasterTime);

    if (mOsaekomi)
        match.setHasAwardedOsaekomiWazari(true);

    auto type = (mOsaekomi ? MatchEventType::WAZARI_OSAEKOMI : MatchEventType::WAZARI);
    match.pushEvent({type, mPlayerIndex, match.currentDuration(mMasterTime)});
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

void MatchEventAction::save(const MatchStore &match, unsigned int eventsToSave ) {
    mDidSave = true;
    mPrevStatus = match.getStatus();
    mPrevWhiteScore = match.getScore(MatchStore::PlayerIndex::WHITE);
    mPrevBlueScore = match.getScore(MatchStore::PlayerIndex::BLUE);
    mPrevGoldenScore = match.isGoldenScore();
    mPrevResumeTime = match.getResumeTime();
    mPrevDuration = match.getDuration();
    mPrevEventSize = match.getEvents().size();
    mPrevBye = match.isBye();
    mPrevOsaekomi = match.getOsaekomi();
    mPrevHasAwardedOsaekomiWazari = match.hasAwardedOsaekomiWazari();

    const auto &events = match.getEvents();
    eventsToSave = std::min(eventsToSave, static_cast<unsigned int>(events.size()));
    for (auto i = events.rbegin(); i != events.rbegin() + eventsToSave; ++i)
        mSavedEvents.push_back(*i);
}

void MatchEventAction::recover(TournamentStore &tournament) {
    assert(mDidSave);

    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    auto &category = tournament.getCategory(mCategoryId);
    auto &match = category.getMatch(mMatchId);
    auto updatedStatus = match.getStatus();

    match.setStatus(mPrevStatus);
    match.getScore(MatchStore::PlayerIndex::WHITE) = mPrevWhiteScore;
    match.getScore(MatchStore::PlayerIndex::BLUE) = mPrevBlueScore;

    match.setGoldenScore(mPrevGoldenScore);
    match.setResumeTime(mPrevResumeTime);
    match.setDuration(mPrevDuration);
    match.setBye(mPrevBye);

    match.setHasAwardedOsaekomiWazari(mPrevHasAwardedOsaekomiWazari);
    match.setOsaekomi(mPrevOsaekomi);

    if (match.getEvents().size() < mPrevEventSize) {
        assert(mPrevEventSize - match.getEvents().size() <= mSavedEvents.size());
        while (mPrevEventSize - match.getEvents().size() > mSavedEvents.size())
            match.popEvent(); // pop added events

        for (auto &event : mSavedEvents)
            match.pushEvent(event);
        mSavedEvents.clear();
    }
    else {
        // pop events if neccesary
        while (match.getEvents().size() > mPrevEventSize)
            match.popEvent();
    }

    // Updates tatami groups
    auto blockLocation = category.getLocation(match.getType());

    if (blockLocation && updatedStatus != mPrevStatus) {
        auto &categoryStatus = category.getStatus(match.getType());

        if (updatedStatus == MatchStatus::NOT_STARTED) {
            assert(categoryStatus.notStartedMatches > 0);
            --(categoryStatus.notStartedMatches);
        }
        else if (updatedStatus == MatchStatus::PAUSED || updatedStatus == MatchStatus::UNPAUSED) {
            assert(categoryStatus.startedMatches > 0);
            --(categoryStatus.startedMatches);
        }
        else if (updatedStatus == MatchStatus::FINISHED) {
            assert(categoryStatus.finishedMatches > 0);
            --(categoryStatus.finishedMatches);
        }

        if (mPrevStatus == MatchStatus::NOT_STARTED)
            ++(categoryStatus.notStartedMatches);
        else if (mPrevStatus == MatchStatus::PAUSED || mPrevStatus == MatchStatus::UNPAUSED)
            ++(categoryStatus.startedMatches);
        else if (mPrevStatus == MatchStatus::FINISHED)
            ++(categoryStatus.finishedMatches);

        auto &concurrentGroup = tournament.getTatamis().at(blockLocation->sequentialGroup.concurrentGroup);
        concurrentGroup.updateStatus(match);

        std::pair<CategoryId, MatchType> block{category.getId(), match.getType()};
        tournament.changeTatamis({*blockLocation}, {block});
    }

    // Notify of match changed
    tournament.changeMatches(match.getCategory(), {match.getId()});

    // Notify results
    if (updatedStatus == MatchStatus::FINISHED || mPrevStatus == MatchStatus::FINISHED)
        tournament.resetCategoryResults(match.getCategory());
}

bool MatchEventAction::shouldRecover() {
    return mDidSave;
}

void MatchEventAction::notify(TournamentStore &tournament, const MatchStore &match) {
    auto &category = tournament.getCategory(match.getCategory());

    // update category status
    auto & categoryStatus = category.getStatus(match.getType());
    if (mPrevStatus == MatchStatus::NOT_STARTED) {
        assert(categoryStatus.notStartedMatches > 0);
        --(categoryStatus.notStartedMatches);
    }
    else if (mPrevStatus == MatchStatus::PAUSED || mPrevStatus == MatchStatus::UNPAUSED) {
        assert(categoryStatus.startedMatches > 0);
        --(categoryStatus.startedMatches);
    }
    else if (mPrevStatus == MatchStatus::FINISHED) {
        assert(categoryStatus.finishedMatches > 0);
        --(categoryStatus.finishedMatches);
    }

    if (match.getStatus() == MatchStatus::NOT_STARTED)
        ++(categoryStatus.notStartedMatches);
    else if (match.getStatus() == MatchStatus::PAUSED || match.getStatus() == MatchStatus::UNPAUSED)
        ++(categoryStatus.startedMatches);
    else if (match.getStatus() == MatchStatus::FINISHED)
        ++(categoryStatus.finishedMatches);

    // Updates tatami groups
    auto blockLocation = category.getLocation(match.getType());
    if (blockLocation && match.getStatus() != mPrevStatus) {
        auto &concurrentGroup = tournament.getTatamis().at(blockLocation->sequentialGroup.concurrentGroup);
        concurrentGroup.updateStatus(match);

        std::pair<CategoryId, MatchType> block{category.getId(), match.getType()};
        tournament.changeTatamis({*blockLocation}, {block});
    }

    // Notify draw system
    // Changes to draws can only occur if the match was finished or is finished
    if (mPrevStatus == MatchStatus::FINISHED || match.getStatus() == MatchStatus::FINISHED) {
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }

        tournament.resetCategoryResults(match.getCategory());
    }

    // Notify of match changed
    tournament.changeMatches(match.getCategory(), {match.getId()});
}

SetMatchPlayerAction::SetMatchPlayerAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::optional<PlayerId> playerId)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
    , mPlayerIndex(playerIndex)
    , mPlayerId(playerId)
{}

void SetMatchPlayerAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    if (mPlayerId.has_value() && !tournament.containsPlayer(*mPlayerId))
        return;

    mOldPlayerId = match.getPlayer(mPlayerIndex);
    match.setPlayer(mPlayerIndex, mPlayerId);

    if (mPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mPlayerId);
        player.addMatch(mCategoryId, mMatchId);
    }

    if (mOldPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mOldPlayerId);
        player.eraseMatch(mCategoryId, mMatchId);
    }

    tournament.changeMatches(mCategoryId, {mMatchId});
    if (mPlayerId.has_value())
        tournament.addMatchesToPlayer(*mPlayerId, {std::make_pair(mCategoryId, mMatchId)});
    if (mOldPlayerId.has_value())
        tournament.eraseMatchesFromPlayer(*mPlayerId, {std::make_pair(mCategoryId, mMatchId)});
}

void SetMatchPlayerAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    if (mPlayerId.has_value() && !tournament.containsPlayer(*mPlayerId))
        return;

    match.setPlayer(mPlayerIndex, mOldPlayerId);

    if (mOldPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mOldPlayerId);
        player.addMatch(mCategoryId, mMatchId);
    }

    if (mPlayerId.has_value()) {
        auto &player = tournament.getPlayer(*mPlayerId);
        player.eraseMatch(mCategoryId, mMatchId);
    }

    tournament.changeMatches(mCategoryId, {mMatchId});
    if (mPlayerId.has_value())
        tournament.eraseMatchesFromPlayer(*mPlayerId, {std::make_pair(mCategoryId, mMatchId)});
    if (mOldPlayerId.has_value())
        tournament.addMatchesToPlayer(*mPlayerId, {std::make_pair(mCategoryId, mMatchId)});
}

std::unique_ptr<Action> SetMatchPlayerAction::freshClone() const {
    return std::make_unique<SetMatchPlayerAction>(mCategoryId, mMatchId, mPlayerIndex, mPlayerId);
}

std::string SetMatchPlayerAction::getDescription() const {
    if (mPlayerIndex == MatchStore::PlayerIndex::WHITE)
        return "Set white match player";
    return "Set blue match player";
}

SetMatchByeAction::SetMatchByeAction(CategoryId categoryId, MatchId matchId, bool bye)
    : MatchEventAction(categoryId, matchId)
    , mBye(bye)
{}

void SetMatchByeAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

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
    return std::make_unique<SetMatchByeAction>(mCategoryId, mMatchId, mBye);
}

std::string SetMatchByeAction::getDescription() const {
    return "Set match bye status";
}

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
    notify(tournament, match);
}

void StartOsaekomiAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

StopOsaekomiAction::StopOsaekomiAction(CategoryId categoryId, MatchId matchId, std::chrono::milliseconds masterTime)
    : MatchEventAction(categoryId, matchId)
    , mMasterTime(masterTime)
{}

std::unique_ptr<Action> StopOsaekomiAction::freshClone() const {
    return std::make_unique<StopOsaekomiAction>(mCategoryId, mMatchId, mMasterTime);
}

std::string StopOsaekomiAction::getDescription() const {
    return "Stop Osaekomi";
}

void StopOsaekomiAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;

    auto &match = category.getMatch(mMatchId);
    const auto &ruleset = category.getRuleset();

    if (!ruleset.canStopOsaekomi(match, mMasterTime))
        return;

    save(match);
    ruleset.stopOsaekomi(match, mMasterTime);
    notify(tournament, match);
}

void StopOsaekomiAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

ResetMatchAction::ResetMatchAction(CategoryId categoryId, MatchId matchId)
    : MatchEventAction(categoryId, matchId)
{}

std::unique_ptr<Action> ResetMatchAction::freshClone() const {
    return std::make_unique<ResetMatchAction>(mCategoryId, mMatchId);
}

std::string ResetMatchAction::getDescription() const {
    return "Reset match";
}

void ResetMatchAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    save(match, match.getEvents().size()); // save all events as well
    match.clear();

    notify(tournament, match);
}

void ResetMatchAction::undoImpl(TournamentStore & tournament) {
    if (shouldRecover())
        recover(tournament);
}

bool ResetMatchAction::shouldDisplay(CategoryId categoryId, MatchId matchId) const {
    return (mCategoryId == categoryId && mMatchId == matchId);
}

