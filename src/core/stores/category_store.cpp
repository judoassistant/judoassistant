#include <sstream>

#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"

CategoryStore::CategoryStore(CategoryId id, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawSystem> drawSystem)
    : mId(id)
    , mName(name)
    , mMatchCount({0,0})
    , mLocation({std::nullopt,std::nullopt})
    , mRuleset(std::move(ruleset))
    , mDrawSystem(std::move(drawSystem))
    , mDrawDisabled(false)
{}

CategoryStore::CategoryStore(const CategoryStore &other)
    : mId(other.mId)
    , mName(other.mName)
    , mPlayers(other.mPlayers)
    , mMatchMap(other.mMatchMap)
    , mMatchCount(other.mMatchCount)
    , mStatus(other.mStatus)
    , mLocation(other.mLocation)
    , mRuleset(other.mRuleset->clone())
    , mDrawSystem(other.mDrawSystem->clone())
    , mDrawDisabled(other.mDrawDisabled)
{
    for (const auto & match : other.mMatches)
        mMatches.push_back(std::make_unique<MatchStore>(*match));
}

const std::unordered_set<PlayerId> & CategoryStore::getPlayers() const {
    return mPlayers;
}

void CategoryStore::erasePlayer(PlayerId id) {
    auto it = mPlayers.find(id);
    assert(it != mPlayers.end());
    mPlayers.erase(it);
}

void CategoryStore::addPlayer(PlayerId id) {
    assert(mPlayers.find(id) == mPlayers.end());
    mPlayers.insert(id);
}

const std::string & CategoryStore::getName() const {
    return mName;
}

void CategoryStore::setName(const std::string &name) {
    mName = name;
}

const CategoryId & CategoryStore::getId() const {
    return mId;
}

std::unique_ptr<Ruleset> CategoryStore::setRuleset(std::unique_ptr<Ruleset> ptr) {
    auto old = std::move(mRuleset);
    mRuleset = std::move(ptr);
    return old;
}

Ruleset & CategoryStore::getRuleset() {
    return *mRuleset;
}

const Ruleset & CategoryStore::getRuleset() const {
    return *mRuleset;
}

std::unique_ptr<DrawSystem> CategoryStore::setDrawSystem(std::unique_ptr<DrawSystem> ptr) {
    auto old = std::move(mDrawSystem);
    mDrawSystem = std::move(ptr);
    return old;
}

DrawSystem & CategoryStore::getDrawSystem() {
    return *mDrawSystem;
}

const DrawSystem & CategoryStore::getDrawSystem() const {
    return *mDrawSystem;
}

bool CategoryStore::containsPlayer(PlayerId id) const {
    return mPlayers.find(id) != mPlayers.end();
}

const CategoryStore::MatchList & CategoryStore::getMatches() const {
    return mMatches;
}

CategoryStore::MatchList & CategoryStore::getMatches() {
    return mMatches;
}

MatchStore & CategoryStore::getMatch(MatchId id) {
    auto it = mMatchMap.find(id);
    assert(it != mMatchMap.end());
    return *(mMatches[it->second]);
}

const MatchStore & CategoryStore::getMatch(MatchId id) const {
    auto it = mMatchMap.find(id);
    assert(it != mMatchMap.end());
    return *(mMatches[it->second]);
}

void CategoryStore::pushMatch(std::unique_ptr<MatchStore> match) {
    MatchId id = match->getId();

    if (!match->isPermanentBye())
        ++(mMatchCount[static_cast<int>(match->getType())]);

    mMatches.push_back(std::move(match));
    assert(mMatchMap.find(id) == mMatchMap.end());
    mMatchMap[id] = mMatches.size() - 1;
}

std::unique_ptr<MatchStore> CategoryStore::popMatch() {
    std::unique_ptr<MatchStore> match = std::move(mMatches.back());
    mMatches.pop_back();

    mMatchMap.erase(match->getId());
    --(mMatchCount[static_cast<int>(match->getType())]);

    return match;
}

bool CategoryStore::containsMatch(MatchId id) const {
    return mMatchMap.find(id) != mMatchMap.end();
}

CategoryStore::MatchList CategoryStore::clearMatches() {
    MatchList res = std::move(mMatches);
    mMatches.clear();
    mMatchMap.clear();
    mMatchCount[0] = 0;
    mMatchCount[1] = 0;

    return res;
}

size_t CategoryStore::getMatchCount(MatchType type) const {
    return mMatchCount[static_cast<int>(type)];
}

std::optional<BlockLocation> CategoryStore::getLocation(MatchType type) const {
    return mLocation[static_cast<int>(type)];
}

void CategoryStore::setLocation(MatchType type, std::optional<BlockLocation> location) {
    mLocation[static_cast<int>(type)] = location;
}

const CategoryStatus& CategoryStore::getStatus(MatchType type) const {
    return mStatus[static_cast<size_t>(type)];
}

CategoryStatus& CategoryStore::getStatus(MatchType type) {
    return mStatus[static_cast<size_t>(type)];
}

void CategoryStore::setStatus(MatchType type, const CategoryStatus &status) {
    mStatus[static_cast<size_t>(type)] = status;
}

std::chrono::milliseconds CategoryStore::expectedDuration(MatchType type) const {
    auto expectedDuration = mRuleset->getExpectedTime() * getMatchCount(type);

    if (expectedDuration < MIN_EXPECTED_DURATION)
        return MIN_EXPECTED_DURATION;
    return expectedDuration;
}

CategoryStatus operator+(const CategoryStatus &a, const CategoryStatus &b) {
    CategoryStatus result;
    result.notStartedMatches = a.notStartedMatches + b.notStartedMatches;
    result.startedMatches = a.startedMatches + b.startedMatches;
    result.finishedMatches = a.finishedMatches + b.finishedMatches;
    return result;
}

bool CategoryStatus::isFinished() const {
    return notStartedMatches == 0 && startedMatches == 0;
}

void CategoryStore::setDrawDisabled(bool disabled) {
    mDrawDisabled = disabled;
}

bool CategoryStore::isDrawDisabled() const {
    return mDrawDisabled;
}

