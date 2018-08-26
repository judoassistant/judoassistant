#include "stores/tournament_store.hpp"

TournamentStore::TournamentStore()
    : mNextPlayerId(0)
    , mNextCategoryId(0)
{}

TournamentStore::~TournamentStore() {}

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

Id TournamentStore::generateNextCategoryId() {
    return mNextCategoryId++;
}

Id TournamentStore::generateNextMatchId() {
    return mNextMatchId++;
}

void TournamentStore::addPlayer(std::unique_ptr<PlayerStore> ptr) {
    mPlayers[ptr->getId()] = std::move(ptr);
}

PlayerStore & TournamentStore::getPlayer(Id id) {
    return *mPlayers[id];
}

void TournamentStore::dispatchAction(std::unique_ptr<Action> && action) {
    (*action)(*this);
    mActions.push(std::move(action));
}

CategoryStore & TournamentStore::getCategory(Id id) {
    return *mCategories[id];
}
