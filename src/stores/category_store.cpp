#include "stores/category_store.hpp"

CategoryStore::CategoryStore() {

}

void CategoryStore::addMatch(std::unique_ptr<MatchStore> ptr) {
    mMatches[ptr->getId()] = std::move(ptr);
}

const std::map<Id, std::unique_ptr<MatchStore>> & CategoryStore::getMatches() const {
    return mMatches;
}

