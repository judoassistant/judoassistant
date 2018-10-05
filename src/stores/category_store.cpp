#include "stores/category_store.hpp"

CategoryStore::CategoryStore() {

}

void CategoryStore::addMatch(std::unique_ptr<MatchStore> && ptr) {
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

const Id & CategoryStore::getId() const {
    return mId;
}

void CategoryStore::setRuleset(std::unique_ptr<Ruleset> && ptr) {
    mRuleset = std::move(ptr);
}

Ruleset & CategoryStore::getRuleset() {
    return *mRuleset;
}
