#include "stores/match_store.hpp"
#include "rulesets/ruleset.hpp"

MatchStore::MatchStore(Id id, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer)
    : mId(id)
{
    mPlayers[static_cast<size_t>(PlayerIndex::WHITE)] = whitePlayer;
    mPlayers[static_cast<size_t>(PlayerIndex::BLUE)] = bluePlayer;
}

Id MatchStore::getId() const {
    return mId;
}

void MatchStore::stop() {
    mIsStopped = true;
}

void MatchStore::resume() {
    mIsStopped = false;
}


bool MatchStore::isGoldenScore() const {
    return mGoldenScore;
}

void MatchStore::setGoldenScore(bool val) {
    mGoldenScore = val;
}

bool MatchStore::isStopped() const {
    return mIsStopped;
}

std::optional<Id> MatchStore::getPlayer(PlayerIndex index) const {
    return mPlayers[static_cast<size_t>(index)];
}

void MatchStore::pushEvent(std::unique_ptr<MatchEvent> && event) {
    mEvents.push_back(std::move(event));
}

const std::vector<std::unique_ptr<MatchEvent>> & MatchStore::getEvents() const {
    return mEvents;
}

std::chrono::high_resolution_clock::time_point MatchStore::getTime() const {
    return mTime;
}

std::chrono::high_resolution_clock::duration MatchStore::getClock() const {
    return mClock;
}

std::chrono::high_resolution_clock::duration MatchStore::getCurrentClock() const {
    if (isStopped())
        return mClock;
    else
        return (std::chrono::high_resolution_clock::now() - getTime()) - mClock;
}

void MatchStore::setTime(const std::chrono::high_resolution_clock::time_point & time) {
    mTime = time;
}

void MatchStore::setClock(const std::chrono::high_resolution_clock::duration & clock) {
    mClock = clock;
}


PlayerScore & MatchStore::getPlayerScore(PlayerIndex index) {
    return mScores[static_cast<size_t>(index)];
}

StopMatchEvent::StopMatchEvent(std::chrono::high_resolution_clock::time_point newTime, std::chrono::high_resolution_clock::time_point oldTime, std::chrono::high_resolution_clock::duration newClock, std::chrono::high_resolution_clock::duration oldClock)
    : mNewTime(newTime)
    , mOldTime(oldTime)
    , mNewClock(newClock)
    , mOldClock(oldClock)
{}

StopMatchEvent::StopMatchEvent(std::unique_ptr<MatchStore> & match) {
    mNewTime = std::chrono::high_resolution_clock::now();
    mOldTime = match->getTime();
    mNewClock = match->getClock() + (mNewTime - mOldTime);
    mOldClock = match->getClock();
}

bool StopMatchEvent::operator()(std::unique_ptr<MatchStore> & match, std::unique_ptr<Ruleset> & ruleset) const {
    return ruleset->stop(match, mNewTime, mNewClock);
}

std::unique_ptr<MatchEvent> StopMatchEvent::getInverse() const {
    return std::make_unique<ResumeMatchEvent>(mOldTime, mNewTime, mOldClock, mNewClock);
}

std::unique_ptr<MatchEvent> StopMatchEvent::clone() const {
    return std::make_unique<StopMatchEvent>(*this);
}

ResumeMatchEvent::ResumeMatchEvent(std::unique_ptr<MatchStore> & match) {
    mNewTime = std::chrono::high_resolution_clock::now();
    mOldTime = match->getTime();
    mNewClock = match->getClock() + (mNewTime - mOldTime);
    mOldClock = match->getClock();
}

ResumeMatchEvent::ResumeMatchEvent(std::chrono::high_resolution_clock::time_point newTime, std::chrono::high_resolution_clock::time_point oldTime, std::chrono::high_resolution_clock::duration newClock, std::chrono::high_resolution_clock::duration oldClock)
    : mNewTime(newTime)
    , mOldTime(oldTime)
    , mNewClock(newClock)
    , mOldClock(oldClock)
{}

bool ResumeMatchEvent::operator()(std::unique_ptr<MatchStore> & match, std::unique_ptr<Ruleset> & ruleset) const {
    return ruleset->resume(match, mNewTime, mNewClock);
}

std::unique_ptr<MatchEvent> ResumeMatchEvent::getInverse() const {
    return std::make_unique<StopMatchEvent>(mOldTime, mNewTime, mOldClock, mNewClock);
}

std::unique_ptr<MatchEvent> ResumeMatchEvent::clone() const {
    return std::make_unique<ResumeMatchEvent>(*this);
}

