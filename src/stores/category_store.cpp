#include "stores/category_store.hpp"

CategoryStore::CategoryStore(Id id, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawStrategy> drawStrategy)
    : mId(id)
    , mName(name)
    , mRuleset(std::move(ruleset))
    , mDrawStrategy(std::move(drawStrategy))
    , mIsDrawn(false)
{}

void CategoryStore::addMatch(std::unique_ptr<MatchStore> && ptr) {
    // TODO: Make sure all code has range checks
    auto it = mMatches.find(ptr->getId());
    if (it != mMatches.end())
        throw std::out_of_range("Attempted to add already existing match");
    mMatches[ptr->getId()] = std::move(ptr);
}

const std::map<Id, std::unique_ptr<MatchStore>> & CategoryStore::getMatches() const {
    return mMatches;
}

MatchStore & CategoryStore::getMatch(Id id) {
    auto it = mMatches.find(id);
    if (it == mMatches.end())
        throw std::out_of_range("Attempted to get non-existing match");

    return *(it->second);
}

std::unique_ptr<MatchStore> CategoryStore::eraseMatch(Id id) {
    auto it = mMatches.find(id);
    if (it == mMatches.end())
        throw std::out_of_range("Attempted to erase non-existing match");

    auto ptr = std::move(it->second);
    mMatches.erase(it);
    return ptr;
}

const std::unordered_set<Id> & CategoryStore::getPlayers() const {
    return mPlayers;
}

void CategoryStore::erasePlayer(Id id) {
    auto it = mPlayers.find(id);
    if (it == mPlayers.end())
        throw std::out_of_range("Attempted to erase non-existing player");

    mPlayers.erase(id);
}

void CategoryStore::addPlayer(Id id) {
    auto it = mPlayers.find(id);
    if (it != mPlayers.end())
        throw std::out_of_range("Attempted to add already existing player");

    mPlayers.insert(id);
}

const std::string & CategoryStore::getName() const {
    return mName;
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

void CategoryStore::setDrawStrategy(std::unique_ptr<DrawStrategy> && ptr) {
    mDrawStrategy = std::move(ptr);
}

DrawStrategy & CategoryStore::getDrawStrategy() {
    return *mDrawStrategy;
}

void CategoryStore::setIsDrawn(bool isDrawn) {
    mIsDrawn = isDrawn;
}

bool CategoryStore::isDrawn() {
    return mIsDrawn;
}
