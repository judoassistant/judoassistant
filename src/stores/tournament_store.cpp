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
    auto it = mPlayers.find(id);
    return *(it->second);
}

const PlayerStore & TournamentStore::getPlayer(Id id) const {
    auto it = mPlayers.find(id);
    return *(it->second);
}

std::unique_ptr<PlayerStore> TournamentStore::erasePlayer(Id id) {
    auto it = mPlayers.find(id);
    auto ptr = std::move(it->second);
    mPlayers.erase(it);
    return std::move(ptr);
}

CategoryStore & TournamentStore::getCategory(Id id) {
    auto it = mCategories.find(id);
    return *(it->second);
}

const CategoryStore & TournamentStore::getCategory(Id id) const {
    auto it = mCategories.find(id);
    return *(it->second);
}

void TournamentStore::addCategory(std::unique_ptr<CategoryStore> ptr) {
    mCategories[ptr->getId()] = std::move(ptr);
}

std::unique_ptr<CategoryStore> TournamentStore::eraseCategory(Id id) {
    auto it = mCategories.find(id);
    auto ptr = std::move(it->second);
    mCategories.erase(it);
    return std::move(ptr);
}

bool TournamentStore::containsPlayer(Id id) const {
    return mPlayers.find(id) != mPlayers.end();
}

bool TournamentStore::containsCategory(Id id) const {
    return mCategories.find(id) != mCategories.end();
}
