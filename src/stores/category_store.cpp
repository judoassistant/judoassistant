#include <sstream>

#include "draw_systems/draw_system.hpp"
#include "rulesets/ruleset.hpp"
#include "stores/category_store.hpp"
#include "stores/match_store.hpp"
#include "stores/match_event.hpp"

CategoryStore::CategoryStore(CategoryId id, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawSystem> drawSystem)
    : mId(id)
    , mName(name)
    , mMatchCount({0,0})
    , mTatamiLocation({std::nullopt,std::nullopt})
    , mRuleset(std::move(ruleset))
    , mDrawSystem(std::move(drawSystem))
{}

CategoryStore::CategoryStore(const CategoryStore &other)
    : mId(other.mId)
    , mName(other.mName)
    , mPlayers(other.mPlayers)
    , mMatchMap(other.mMatchMap)
    , mMatchCount(other.mMatchCount)
    , mTatamiLocation(other.mTatamiLocation)
    , mRuleset(other.mRuleset->clone())
    , mDrawSystem(other.mDrawSystem->clone())
{
    for (const auto & match : other.mMatches)
        mMatches.push_back(std::make_unique<MatchStore>(*match));
}

const std::unordered_set<PlayerId> & CategoryStore::getPlayers() const {
    return mPlayers;
}

void CategoryStore::erasePlayer(PlayerId id) {
    mPlayers.erase(id);
}

void CategoryStore::addPlayer(PlayerId id) {
    mPlayers.insert(id);
}

const std::string & CategoryStore::getName() const {
    return mName;
}

std::string CategoryStore::getName(MatchType type) const {
    std::stringstream res;
    res << mName;
    if (getDrawSystem().hasFinalBlock())
        res << " (" << (type == MatchType::KNOCKOUT ? "Knockout" : "Final") << ")"; // TODO: Translate
    return res.str();
}

void CategoryStore::setName(const std::string &name) {
    mName = name;
}

const CategoryId & CategoryStore::getId() const {
    return mId;
}

std::unique_ptr<Ruleset> CategoryStore::setRuleset(std::unique_ptr<Ruleset> && ptr) {
    auto old = std::move(mRuleset);
    mRuleset = std::move(ptr);
    return std::move(old);
}

Ruleset & CategoryStore::getRuleset() {
    return *mRuleset;
}

const Ruleset & CategoryStore::getRuleset() const {
    return *mRuleset;
}

std::unique_ptr<DrawSystem> CategoryStore::setDrawSystem(std::unique_ptr<DrawSystem> && ptr) {
    auto old = std::move(mDrawSystem);
    mDrawSystem = std::move(ptr);
    return std::move(old);
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
    size_t index = mMatchMap.find(id)->second;
    return *(mMatches[index]);
}

const MatchStore & CategoryStore::getMatch(MatchId id) const {
    size_t index = mMatchMap.find(id)->second;
    return *(mMatches[index]);
}

void CategoryStore::pushMatch(std::unique_ptr<MatchStore> &&match) {
    MatchId id = match->getId();

    if (!match->isBye())
        ++(mMatchCount[static_cast<int>(match->getType())]);

    mMatches.push_back(std::move(match));
    mMatchMap[id] = mMatches.size() - 1;
}

std::unique_ptr<MatchStore> CategoryStore::popMatch() {
    std::unique_ptr<MatchStore> match = std::move(mMatches.back());
    mMatches.pop_back();

    mMatchMap.erase(match->getId());
    --(mMatchCount[static_cast<int>(match->getType())]);

    return std::move(match);
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

    return std::move(res);
}

size_t CategoryStore::getMatchCount(MatchType type) const {
    return mMatchCount[static_cast<int>(type)];
}

std::optional<TatamiLocation> CategoryStore::getTatamiLocation(MatchType type) const {
    return mTatamiLocation[static_cast<int>(type)];
}

void CategoryStore::setTatamiLocation(MatchType type, std::optional<TatamiLocation> location) {
    mTatamiLocation[static_cast<int>(type)] = location;
}
