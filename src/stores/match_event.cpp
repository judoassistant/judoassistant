#include "stores/match_event.hpp"
#include "stores/match_store.hpp"
#include "rulesets/ruleset.hpp"

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

