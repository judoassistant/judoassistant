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
    if (mPlayers.find(ptr->getId()) != mPlayers.end())
        throw std::runtime_error("Attempted to create player with non-unique id");
    mPlayers[ptr->getId()] = std::move(ptr);
}

PlayerStore & TournamentStore::getPlayer(Id id) {
    auto it = mPlayers.find(id);
    if (it == mPlayers.end())
        throw std::out_of_range("Attempted to get non-existing player");

    return *(it->second);
}

std::unique_ptr<PlayerStore> TournamentStore::erasePlayer(Id id) {
    auto it = mPlayers.find(id);
    if (it == mPlayers.end())
        throw std::out_of_range("Attempted to erase non-existing player");

    auto ptr = std::move(it->second);
    mPlayers.erase(it);
    return ptr;
}

CategoryStore & TournamentStore::getCategory(Id id) {
    auto it = mCategories.find(id);
    if (it == mCategories.end())
        throw std::out_of_range("Attempted to get non-existing category");

    return *(it->second);
}
