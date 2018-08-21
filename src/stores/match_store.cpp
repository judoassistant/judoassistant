#include "stores/match_store.hpp"
#include "rulesets/ruleset.hpp"
#include "stores/match_event.hpp"

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
