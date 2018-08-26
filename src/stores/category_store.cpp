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
    return *mMatches[id];
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
