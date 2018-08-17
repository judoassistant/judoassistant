#include "stores/match_store.hpp"

const Id & MatchStore::getId() const {
    return mId;
}

const Id & MatchStore::getCategory() const {
    return mCategory;
}

const std::optional<Id> & MatchStore::getWhitePlayer() const {
    return mWhitePlayer;
}

const std::optional<Id> & MatchStore::getBluePlayer() const {
    return mBluePlayer;
}


