#include "rulesets/ruleset.hpp"
#include "stores/match_store.hpp"

MatchStore::MatchStore(MatchId id, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayer, std::optional<PlayerId> bluePlayer)
    : mId(id)
    , mCategory(categoryId)
    , mType(type)
    , mTitle(title)
    , mBye(bye)
    , mStatus(MatchStatus::NOT_STARTED)
    , mGoldenScore(false)
    , mDuration(0)
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

void MatchStore::pushEvent(const MatchEvent & event) {
    mEvents.push_back(event);
}

void MatchStore::popEvent() {
    mEvents.pop_back();
}

const std::vector<MatchEvent> & MatchStore::getEvents() const {
    return mEvents;
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
    , mResumeTime(other.mResumeTime)
    , mDuration(other.mDuration)
    , mEvents(other.mEvents)
{}

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

void MatchStore::setStatus(MatchStatus status) {
    mStatus = status;
}

const std::string & MatchStore::getTitle() const {
    return mTitle;
}

void MatchStore::setDuration(std::chrono::milliseconds duration) {
    mDuration = duration;
}

std::chrono::milliseconds MatchStore::getDuration() const {
    return mDuration;
}

void MatchStore::setResumeTime(std::chrono::milliseconds resumeTime) {
    mResumeTime = resumeTime;
}

std::chrono::milliseconds MatchStore::getResumeTime() const {
    return mResumeTime;
}

std::chrono::milliseconds MatchStore::currentDuration(std::chrono::milliseconds masterTime) const {
    if (mStatus != MatchStatus::UNPAUSED)
        return mDuration;

    return (masterTime - mResumeTime) + mDuration;
}
