#include "stores/category_store.hpp"

CategoryStore::CategoryStore(CategoryId id, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawSystem> drawSystem)
    : mId(id)
    , mName(name)
    , mRuleset(std::move(ruleset))
    , mDrawSystem(std::move(drawSystem))
{}

void CategoryStore::addMatch(std::unique_ptr<MatchStore> && ptr) {
    mMatches[ptr->getId()] = std::move(ptr);
}

const std::map<MatchId, std::unique_ptr<MatchStore>> & CategoryStore::getMatches() const {
    return mMatches;
}

MatchStore & CategoryStore::getMatch(MatchId id) {
    auto it = mMatches.find(id);
    return *(it->second);
}

const MatchStore & CategoryStore::getMatch(MatchId id) const {
    auto it = mMatches.find(id);
    return *(it->second);
}

std::unique_ptr<MatchStore> CategoryStore::eraseMatch(MatchId id) {
    auto it = mMatches.find(id);
    auto ptr = std::move(it->second);
    mMatches.erase(it);
    return std::move(ptr);
}

const std::unordered_set<PlayerId, PlayerId::Hasher> & CategoryStore::getPlayers() const {
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

void CategoryStore::setName(const std::string &name) {
    mName = name;
}

const CategoryId & CategoryStore::getId() const {
    return mId;
}

void CategoryStore::setRuleset(std::unique_ptr<Ruleset> && ptr) {
    mRuleset = std::move(ptr);
}

Ruleset & CategoryStore::getRuleset() {
    return *mRuleset;
}

const Ruleset & CategoryStore::getRuleset() const {
    return *mRuleset;
}

void CategoryStore::setDrawSystem(std::unique_ptr<DrawSystem> && ptr) {
    mDrawSystem = std::move(ptr);
}

DrawSystem & CategoryStore::getDrawSystem() {
    return *mDrawSystem;
}

const DrawSystem & CategoryStore::getDrawSystem() const {
    return *mDrawSystem;
}

bool CategoryStore::containsMatch(MatchId id) const {
    return mMatches.find(id) != mMatches.end();
}

bool CategoryStore::containsPlayer(PlayerId id) const {
    return mPlayers.find(id) != mPlayers.end();
}
