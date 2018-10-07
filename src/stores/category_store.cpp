#include "stores/category_store.hpp"

CategoryStore::CategoryStore(Id id, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawStrategy> drawStrategy)
    : mId(id)
    , mName(name)
    , mRuleset(std::move(ruleset))
    , mDrawStrategy(std::move(drawStrategy))
{}

void CategoryStore::addMatch(std::unique_ptr<MatchStore> && ptr) {
    mMatches[ptr->getId()] = std::move(ptr);
}

const std::map<Id, std::unique_ptr<MatchStore>> & CategoryStore::getMatches() const {
    return mMatches;
}

MatchStore & CategoryStore::getMatch(Id id) {
    auto it = mMatches.find(id);
    return *(it->second);
}

const MatchStore & CategoryStore::getMatch(Id id) const {
    auto it = mMatches.find(id);
    return *(it->second);
}

std::unique_ptr<MatchStore> CategoryStore::eraseMatch(Id id) {
    auto it = mMatches.find(id);
    auto ptr = std::move(it->second);
    mMatches.erase(it);
    return std::move(ptr);
}

const std::unordered_set<Id> & CategoryStore::getPlayers() const {
    return mPlayers;
}

void CategoryStore::erasePlayer(Id id) {
    mPlayers.erase(id);
}

void CategoryStore::addPlayer(Id id) {
    mPlayers.insert(id);
}

const std::string & CategoryStore::getName() const {
    return mName;
}

void CategoryStore::setName(const std::string &name) {
    mName = name;
}

const Id & CategoryStore::getId() const {
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

void CategoryStore::setDrawStrategy(std::unique_ptr<DrawStrategy> && ptr) {
    mDrawStrategy = std::move(ptr);
}

DrawStrategy & CategoryStore::getDrawStrategy() {
    return *mDrawStrategy;
}

const DrawStrategy & CategoryStore::getDrawStrategy() const {
    return *mDrawStrategy;
}

bool CategoryStore::containsMatch(Id id) const {
    return mMatches.find(id) != mMatches.end();
}

bool CategoryStore::containsPlayer(Id id) const {
    return mPlayers.find(id) != mPlayers.end();
}
