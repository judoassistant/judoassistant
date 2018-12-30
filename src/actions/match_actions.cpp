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
    auto &category = tournament.getCategory(mCategoryId);
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
    mPrevGoldenScore = match.isGoldenScore();

    ruleset.pause(match, mMasterTime); // TODO: Update drawing etc. if neccesary
    tournament.changeMatches(mCategoryId, {mMatchId});

    // pausing can only cause draw changes if the match went into a finished state
    assert(mDrawActions.empty());
    if (match.getStatus() == MatchStatus::FINISHED) {
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }
    }
}

void PauseMatchAction::undoImpl(TournamentStore & tournament) {
    if (!mDidPause)
        return;
    auto &category = tournament.getCategory(mCategoryId);
    auto &match = category.getMatch(mMatchId);
    assert(match.getStatus() == MatchStatus::PAUSED);

    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    match.setStatus(MatchStatus::UNPAUSED);
    match.setDuration(mPrevDuration);
    match.setGoldenScore(mPrevGoldenScore);
    tournament.changeMatches(mCategoryId, {mMatchId});
}

AwardIpponAction::AwardIpponAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
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
    mDidAward = false;

    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAddIppon(match, mPlayerIndex))
        return;

    mDidAward = true;
    mPrevStatus = match.getStatus();
    mPrevGoldenScore = match.isGoldenScore();

    ruleset.addIppon(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::IPPON, mPlayerIndex, match.currentDuration(mMasterTime)});
    tournament.changeMatches(mCategoryId, {mMatchId});

    // draw changes only occur when the match is finished or went from finished
    assert(mDrawActions.empty());
    if (match.getStatus() == MatchStatus::FINISHED || mPrevStatus == MatchStatus::FINISHED) {
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }
    }
}

void AwardIpponAction::undoImpl(TournamentStore & tournament) {
    if (!mDidAward)
        return;
    auto &category = tournament.getCategory(mCategoryId);
    auto &match = category.getMatch(mMatchId);
    const auto &ruleset = category.getRuleset();
    assert(ruleset.canSubtractIppon(match, mPlayerIndex));

    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    ruleset.subtractIppon(match, mPlayerIndex, mMasterTime);
    match.setStatus(mPrevStatus);
    match.setGoldenScore(mPrevGoldenScore);

    assert(match.getEvents().back().type == MatchEventType::IPPON);
    match.popEvent();

    tournament.changeMatches(mCategoryId, {mMatchId});
}

AwardWazariAction::AwardWazariAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
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
    mDidAward = false;

    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAddWazari(match, mPlayerIndex))
        return;

    mDidAward = true;
    mPrevStatus = match.getStatus();
    mPrevGoldenScore = match.isGoldenScore();

    ruleset.addWazari(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::WAZARI, mPlayerIndex, match.currentDuration(mMasterTime)});
    tournament.changeMatches(mCategoryId, {mMatchId});

    // draw changes only occur when the match is finished or went from finished
    assert(mDrawActions.empty());
    if (match.getStatus() == MatchStatus::FINISHED || mPrevStatus == MatchStatus::FINISHED) {
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }
    }
}

void AwardWazariAction::undoImpl(TournamentStore & tournament) {
    if (!mDidAward)
        return;
    auto &category = tournament.getCategory(mCategoryId);
    auto &match = category.getMatch(mMatchId);
    const auto &ruleset = category.getRuleset();
    assert(ruleset.canSubtractWazari(match, mPlayerIndex));

    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    ruleset.subtractWazari(match, mPlayerIndex, mMasterTime);
    match.setStatus(mPrevStatus);
    match.setGoldenScore(mPrevGoldenScore);

    assert(match.getEvents().back().type == MatchEventType::WAZARI);
    match.popEvent();

    tournament.changeMatches(mCategoryId, {mMatchId});

    // draw changes only occur when the match is finished or went from finished
    assert(mDrawActions.empty());
    if (match.getStatus() == MatchStatus::FINISHED || mPrevStatus == MatchStatus::FINISHED) {
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }
    }
}

AwardShidoAction::AwardShidoAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
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
    mDidAward = false;

    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAddShido(match, mPlayerIndex))
        return;

    mDidAward = true;
    mPrevStatus = match.getStatus();
    mPrevGoldenScore = match.isGoldenScore();

    ruleset.addShido(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::SHIDO, mPlayerIndex, match.currentDuration(mMasterTime)});
    tournament.changeMatches(mCategoryId, {mMatchId});

    // draw changes only occur when the match is finished or went from finished
    assert(mDrawActions.empty());
    if (match.getStatus() == MatchStatus::FINISHED || mPrevStatus == MatchStatus::FINISHED) {
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }
    }
}

void AwardShidoAction::undoImpl(TournamentStore & tournament) {
    if (!mDidAward)
        return;
    auto &category = tournament.getCategory(mCategoryId);
    auto &match = category.getMatch(mMatchId);
    const auto &ruleset = category.getRuleset();
    assert(ruleset.canSubtractShido(match, mPlayerIndex));

    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    ruleset.subtractShido(match, mPlayerIndex, mMasterTime);
    match.setStatus(mPrevStatus);
    match.setGoldenScore(mPrevGoldenScore);

    assert(match.getEvents().back().type == MatchEventType::HANSOKU_MAKE);
    match.popEvent();

    tournament.changeMatches(mCategoryId, {mMatchId});
}

AwardHansokuMakeAction::AwardHansokuMakeAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::chrono::milliseconds masterTime)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
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
    mDidAward = false;

    if (!tournament.containsCategory(mCategoryId))
        return;
    auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;
    auto &match = category.getMatch(mMatchId);

    const auto &ruleset = category.getRuleset();
    if (!ruleset.canAddHansokuMake(match, mPlayerIndex))
        return;

    mDidAward = true;
    mPrevStatus = match.getStatus();
    mPrevGoldenScore = match.isGoldenScore();

    ruleset.addHansokuMake(match, mPlayerIndex, mMasterTime);
    match.pushEvent({MatchEventType::HANSOKU_MAKE, mPlayerIndex, match.currentDuration(mMasterTime)});
    tournament.changeMatches(mCategoryId, {mMatchId});

    // draw changes only occur when the match is finished or went from finished
    assert(mDrawActions.empty());
    if (match.getStatus() == MatchStatus::FINISHED || mPrevStatus == MatchStatus::FINISHED) {
        const auto &drawSystem = category.getDrawSystem();
        auto drawActions = drawSystem.updateCategory(tournament, category);
        for (std::unique_ptr<Action> &action : drawActions) {
            action->redo(tournament);
            mDrawActions.push(std::move(action));
        }
    }
}

void AwardHansokuMakeAction::undoImpl(TournamentStore & tournament) {
    if (!mDidAward)
        return;
    auto &category = tournament.getCategory(mCategoryId);
    auto &match = category.getMatch(mMatchId);
    const auto &ruleset = category.getRuleset();
    assert(ruleset.canSubtractHansokuMake(match, mPlayerIndex));

    while (!mDrawActions.empty()) {
        mDrawActions.top()->undo(tournament);
        mDrawActions.pop();
    }

    ruleset.subtractHansokuMake(match, mPlayerIndex, mMasterTime);
    match.setStatus(mPrevStatus);
    match.setGoldenScore(mPrevGoldenScore);

    assert(match.getEvents().back().type == MatchEventType::SHIDO);
    match.popEvent();

    tournament.changeMatches(mCategoryId, {mMatchId});
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

