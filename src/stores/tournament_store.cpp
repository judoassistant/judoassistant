#include "stores/tournament_store.hpp"

TournamentStore::TournamentStore()
{}

TournamentStore::~TournamentStore() {}

const std::string & TournamentStore::getName() const {
    return mName;
}

void TournamentStore::setName(const std::string & name) {
    mName = name;
}

const std::unordered_map<PlayerId, std::unique_ptr<PlayerStore>> & TournamentStore::getPlayers() const {
    return mPlayers;
}

const std::unordered_map<CategoryId, std::unique_ptr<CategoryStore>> & TournamentStore::getCategories() const {
    return mCategories;
}

PlayerId TournamentStore::generateNextPlayerId() {
    while (true) {
        PlayerId id = mPlayerIdGenerator();
        if (!containsPlayer(id))
            return id;
    }
}

CategoryId TournamentStore::generateNextCategoryId() {
    while (true) {
        CategoryId id = mCategoryIdGenerator();
        if (!containsCategory(id))
            return id;
    }
}

MatchId TournamentStore::generateNextMatchId() {
    while (true) {
        MatchId id = mMatchIdGenerator();

        bool exists = false;
        for (const auto & it : getCategories()) {
            if (it.second->containsMatch(id)) {
                exists = true;
                break;
            }
        }

        if (!exists)
            return id;
    }
}

void TournamentStore::addPlayer(std::unique_ptr<PlayerStore> ptr) {
    mPlayers[ptr->getId()] = std::move(ptr);
}

PlayerStore & TournamentStore::getPlayer(PlayerId id) {
    auto it = mPlayers.find(id);
    return *(it->second);
}

const PlayerStore & TournamentStore::getPlayer(PlayerId id) const {
    auto it = mPlayers.find(id);
    return *(it->second);
}

std::unique_ptr<PlayerStore> TournamentStore::erasePlayer(PlayerId id) {
    auto it = mPlayers.find(id);
    auto ptr = std::move(it->second);
    mPlayers.erase(it);
    return std::move(ptr);
}

CategoryStore & TournamentStore::getCategory(CategoryId id) {
    auto it = mCategories.find(id);
    return *(it->second);
}

const CategoryStore & TournamentStore::getCategory(CategoryId id) const {
    auto it = mCategories.find(id);
    return *(it->second);
}

void TournamentStore::addCategory(std::unique_ptr<CategoryStore> ptr) {
    mCategories[ptr->getId()] = std::move(ptr);
}

std::unique_ptr<CategoryStore> TournamentStore::eraseCategory(CategoryId id) {
    auto it = mCategories.find(id);
    auto ptr = std::move(it->second);
    mCategories.erase(it);
    return std::move(ptr);
}

bool TournamentStore::containsPlayer(PlayerId id) const {
    return mPlayers.find(id) != mPlayers.end();
}

bool TournamentStore::containsCategory(CategoryId id) const {
    return mCategories.find(id) != mCategories.end();
}

const TatamiList & TournamentStore::getTatamis() const {
    return mTatamis;
}

TatamiList & TournamentStore::getTatamis() {
    return mTatamis;
}

