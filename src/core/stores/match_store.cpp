#include "core/rulesets/ruleset.hpp"
#include "core/stores/match_store.hpp"

MatchStore::State::State(bool finished)
    : status(finished ? MatchStatus::FINISHED : MatchStatus::NOT_STARTED)
    , goldenScore(false)
    , duration(std::chrono::seconds(0))
    , hasAwardedOsaekomiWazari(false)
{

}

MatchStore::MatchStore(MatchId id, CategoryId categoryId, MatchType type, const std::string &title, bool permanentBye, std::optional<PlayerId> whitePlayer, std::optional<PlayerId> bluePlayer)
    : mId(id)
    , mCategory(categoryId)
    , mType(type)
    , mTitle(title)
    , mPermanentBye(permanentBye)
    , mBye(permanentBye)
    , mState(permanentBye)
{
    mPlayers[static_cast<size_t>(PlayerIndex::WHITE)] = whitePlayer;
    mPlayers[static_cast<size_t>(PlayerIndex::BLUE)] = bluePlayer;
}

MatchStore::MatchStore(const MatchStore &other)
    : mId(other.mId)
    , mCategory(other.mCategory)
    , mType(other.mType)
    , mTitle(other.mTitle)
    , mPermanentBye(other.mPermanentBye)
    , mBye(other.mBye)
    , mPlayers(other.mPlayers)
    , mState(other.mState)
    , mEvents(other.mEvents)
{}

MatchId MatchStore::getId() const {
    return mId;
}

bool MatchStore::isGoldenScore() const {
    return mState.goldenScore;
}

void MatchStore::setGoldenScore(bool val) {
    mState.goldenScore = val;
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

const std::vector<MatchEvent>& MatchStore::getEvents() const {
    return mEvents;
}

std::vector<MatchEvent>& MatchStore::getEvents() {
    return mEvents;
}

MatchStore::Score & MatchStore::getScore(PlayerIndex index) {
    return mState.scores[static_cast<size_t>(index)];
}

const MatchStore::Score & MatchStore::getScore(PlayerIndex index) const {
    return mState.scores[static_cast<size_t>(index)];
}

CategoryId MatchStore::getCategory() const {
    return mCategory;
}

std::pair<CategoryId, MatchId> MatchStore::getCombinedId() const {
    return {mCategory, mId};
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

void MatchStore::setBye(bool bye) {
    assert(!mPermanentBye);
    mBye = bye;
}

bool MatchStore::isPermanentBye() const {
    return mPermanentBye;
}

MatchStore::Score::Score()
    : ippon(0)
    , wazari(0)
    , shido(0)
    , hansokuMake(0)
{}

void MatchStore::finish() {
    mState.status = MatchStatus::FINISHED;
}

MatchStatus MatchStore::getStatus() const {
    return mState.status;
}

void MatchStore::setStatus(MatchStatus status) {
    mState.status = status;
}

const std::string & MatchStore::getTitle() const {
    return mTitle;
}

void MatchStore::setDuration(std::chrono::milliseconds duration) {
    mState.duration = duration;
}

std::chrono::milliseconds MatchStore::getDuration() const {
    return mState.duration;
}

void MatchStore::setResumeTime(std::chrono::milliseconds resumeTime) {
    mState.resumeTime = resumeTime;
}

std::chrono::milliseconds MatchStore::getResumeTime() const {
    return mState.resumeTime;
}

std::chrono::milliseconds MatchStore::currentDuration(std::chrono::milliseconds masterTime) const {
    if (mState.status != MatchStatus::UNPAUSED)
        return mState.duration;

    return (masterTime - mState.resumeTime) + mState.duration;
}

void MatchStore::setPlayer(PlayerIndex index, std::optional<PlayerId> playerId) {
    mPlayers[static_cast<size_t>(index)] = playerId;
}

const std::optional<std::pair<MatchStore::PlayerIndex, std::chrono::milliseconds>>& MatchStore::getOsaekomi() const {
    return mState.osaekomi;
}

void MatchStore::setOsaekomi(const std::optional<std::pair<MatchStore::PlayerIndex, std::chrono::milliseconds>>& value) {
    mState.osaekomi = value;
}

bool MatchStore::hasAwardedOsaekomiWazari() const {
    return mState.hasAwardedOsaekomiWazari;
}

void MatchStore::setHasAwardedOsaekomiWazari(bool val) {
    mState.hasAwardedOsaekomiWazari = val;
}

std::chrono::milliseconds MatchStore::currentOsaekomiTime(std::chrono::milliseconds masterTime) const {
    assert(mState.osaekomi.has_value());

    return (masterTime - mState.osaekomi->second);
}

const MatchStore::State& MatchStore::getState() const {
    return mState;
}

MatchStore::State& MatchStore::getState() {
    return mState;
}

void MatchStore::setState(const MatchStore::State &state) {
    mState = state;
}

void MatchStore::clearEvents() {
    mEvents.clear();
}

void MatchStore::setEvents(const std::vector<MatchEvent> &events) {
    mEvents = events;
}

void MatchStore::clearState() {
    mState = State(mBye);
}

