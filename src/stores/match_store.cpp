#include "stores/match_store.hpp"
#include "rulesets/ruleset.hpp"

MatchStore::MatchStore(Id id, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer)
    : mId(id)
{
    mPlayers[PlayerIndex::WHITE] = whitePlayer;
    mPlayers[PlayerIndex::BLUE] = bluePlayer;
}

Id MatchStore::getId() const {
    return mId;
}

std::optional<Id> MatchStore::getPlayer(PlayerIndex index) const {
    return mPlayers[index];
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

void MatchStore::setTime(const std::chrono::high_resolution_clock::time_point & time) {
    mTime = time;
}

void MatchStore::setClock(const std::chrono::high_resolution_clock::duration & clock) {
    mClock = clock;
}

PauseMatchEvent::PauseMatchEvent(std::chrono::high_resolution_clock::time_point newTime, std::chrono::high_resolution_clock::time_point oldTime, std::chrono::high_resolution_clock::duration newClock, std::chrono::high_resolution_clock::duration oldClock)
    : mNewTime(newTime)
    , mOldTime(oldTime)
    , mNewClock(newClock)
    , mOldClock(oldClock)
{}

PauseMatchEvent::PauseMatchEvent(std::unique_ptr<MatchStore> & match) {
    mNewTime = std::chrono::high_resolution_clock::now();
    mOldTime = match->getTime();
    mNewClock = match->getClock() + (mNewTime - mOldTime);
    mOldClock = match->getClock();
}

bool PauseMatchEvent::operator()(std::unique_ptr<MatchStore> & match, std::unique_ptr<Ruleset> & ruleset) const {
    return ruleset->pause(match, mNewTime, mNewClock);
}

std::unique_ptr<MatchEvent> PauseMatchEvent::getInverse() const {
    return std::make_unique<ResumeMatchEvent>(mOldTime, mNewTime, mOldClock, mNewClock);
}

std::unique_ptr<MatchEvent> PauseMatchEvent::clone() const {
    return std::make_unique<PauseMatchEvent>(*this);
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
    return std::make_unique<PauseMatchEvent>(mOldTime, mNewTime, mOldClock, mNewClock);
}

std::unique_ptr<MatchEvent> ResumeMatchEvent::clone() const {
    return std::make_unique<ResumeMatchEvent>(*this);
}

