#include "stores/match_store.hpp"

MatchStore::MatchStore(Id id, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer)
    : mId(id)
    , mWhitePlayer(whitePlayer)
    , mBluePlayer(bluePlayer)
{}

Id MatchStore::getId() const {
    return mId;
}

std::optional<Id> MatchStore::getWhitePlayer() const {
    return mWhitePlayer;
}

std::optional<Id> MatchStore::getBluePlayer() const {
    return mBluePlayer;
}

bool MatchStore::isFinished() const {
    // TODO
    return false;
}
