#include "stores/tournament_store.hpp"

TournamentStore::TournamentStore()
    : mNextPlayerId(0)
    , mNextCategoryId(0)
{}


const std::string & TournamentStore::getName() const {
    return mName;
}

void TournamentStore::setName(const std::string & name) {
    mName = name;
}

const std::unordered_map<Id, std::unique_ptr<PlayerStore>> & TournamentStore::getPlayers() const {
    return mPlayers;
}

const std::unordered_map<Id, std::unique_ptr<CategoryStore>> & TournamentStore::getCategories() const {
    return mCategories;
}

Id TournamentStore::generateNextPlayerId() {
    return mNextPlayerId++;
}

void TournamentStore::addPlayer(std::unique_ptr<PlayerStore> ptr) {
    mPlayers[ptr->getId()] = std::move(ptr);
}

void TournamentStore::dispatchAction(std::unique_ptr<Action> action) {
    (*action)(this);
    mActions.push(std::move(action));
}
