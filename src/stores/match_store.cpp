#include "rulesets/ruleset.hpp"
#include "stores/match_store.hpp"
#include "stores/match_event.hpp"

MatchStore::MatchStore(MatchId id, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayer, std::optional<PlayerId> bluePlayer)
    : mId(id)
    , mCategory(categoryId)
    , mType(type)
    , mTitle(title)
    , mBye(bye)
    , mStatus(MatchStatus::NOT_STARTED)
    , mGoldenScore(false)
{
    mPlayers[static_cast<size_t>(PlayerIndex::WHITE)] = whitePlayer;
    mPlayers[static_cast<size_t>(PlayerIndex::BLUE)] = bluePlayer;
}

MatchId MatchStore::getId() const {
    return mId;
}

bool MatchStore::isGoldenScore() const {
    return mGoldenScore;
}

void MatchStore::setGoldenScore(bool val) {
    mGoldenScore = val;
}

std::optional<PlayerId> MatchStore::getPlayer(PlayerIndex index) const {
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
    if (mStatus == MatchStatus::UNPAUSED)
        return (std::chrono::high_resolution_clock::now() - getTime()) - mClock;
    else
        return mClock;
}

void MatchStore::setTime(const std::chrono::high_resolution_clock::time_point & time) {
    mTime = time;
}

void MatchStore::setClock(const std::chrono::high_resolution_clock::duration & clock) {
    mClock = clock;
}

MatchStore::Score & MatchStore::getScore(PlayerIndex index) {
    return mScores[static_cast<size_t>(index)];
}

const MatchStore::Score & MatchStore::getScore(PlayerIndex index) const {
    return mScores[static_cast<size_t>(index)];
}

CategoryId MatchStore::getCategory() const {
    return mCategory;
}

MatchStore::Score & MatchStore::getWhiteScore() {
    return getScore(PlayerIndex::WHITE);
}

MatchStore::Score & MatchStore::getBlueScore() {
    return getScore(PlayerIndex::BLUE);
}

const MatchStore::Score & MatchStore::getWhiteScore() const {
    return getScore(PlayerIndex::WHITE);
}

const MatchStore::Score & MatchStore::getBlueScore() const {
    return getScore(PlayerIndex::BLUE);
}

std::optional<PlayerId> MatchStore::getWhitePlayer() const {
    return getPlayer(PlayerIndex::WHITE);
}

std::optional<PlayerId> MatchStore::getBluePlayer() const {
    return getPlayer(PlayerIndex::BLUE);
}

MatchType MatchStore::getType() const {
    return mType;
}

std::ostream &operator<<(std::ostream &out, const MatchType &matchType) {
    if (matchType == MatchType::FINAL)
        return out << "final";
    return out << "knockout";

}

bool MatchStore::isBye() const {
    return mBye;
}

MatchStore::MatchStore(const MatchStore &other)
    : mId(other.mId)
    , mCategory(other.mCategory)
    , mType(other.mType)
    , mTitle(other.mTitle)
    , mBye(other.mBye)
    , mScores(other.mScores)
    , mPlayers(other.mPlayers)
    , mStatus(other.mStatus)
    , mGoldenScore(other.mGoldenScore)
    , mTime(other.mTime)
    , mClock(other.mClock)
{
    for (const auto &event : other.mEvents)
        mEvents.push_back(event->clone());
}

MatchStore::Score::Score()
    : ippon(0)
    , wazari(0)
    , shido(0)
    , hansokuMake(0)
{}

void MatchStore::finish() {
    mStatus = MatchStatus::FINISHED;
}

MatchStatus MatchStore::getStatus() const {
    return mStatus;
}

MatchStatus MatchStore::setStatus(MatchStatus status) {
    mStatus = status;
}

