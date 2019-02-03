#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/tournament_store.hpp"

TournamentStore::TournamentStore()
{}

TournamentStore::TournamentStore(TournamentId id)
    : mId(id)
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

void TournamentStore::addPlayer(std::unique_ptr<PlayerStore> ptr) {
    assert(mPlayers.find(ptr->getId()) == mPlayers.end());
    mPlayers[ptr->getId()] = std::move(ptr);
}

PlayerStore & TournamentStore::getPlayer(PlayerId id) {
    auto it = mPlayers.find(id);
    assert(it != mPlayers.end());
    return *(it->second);
}

const PlayerStore & TournamentStore::getPlayer(PlayerId id) const {
    auto it = mPlayers.find(id);
    assert(it != mPlayers.end());
    return *(it->second);
}

std::unique_ptr<PlayerStore> TournamentStore::erasePlayer(PlayerId id) {
    auto it = mPlayers.find(id);
    assert(it != mPlayers.end());
    auto ptr = std::move(it->second);
    mPlayers.erase(it);
    return std::move(ptr);
}

CategoryStore & TournamentStore::getCategory(CategoryId id) {
    auto it = mCategories.find(id);
    assert(it != mCategories.end());
    return *(it->second);
}

const CategoryStore & TournamentStore::getCategory(CategoryId id) const {
    auto it = mCategories.find(id);
    assert(it != mCategories.end());
    return *(it->second);
}

void TournamentStore::addCategory(std::unique_ptr<CategoryStore> ptr) {
    assert(mCategories.find(ptr->getId()) == mCategories.end());
    mCategories[ptr->getId()] = std::move(ptr);
}

std::unique_ptr<CategoryStore> TournamentStore::eraseCategory(CategoryId id) {
    auto it = mCategories.find(id);
    assert(it != mCategories.end());
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

bool TournamentStore::containsMatch(CategoryId categoryId, MatchId matchId) const {
    auto it = mCategories.find(categoryId);
    if (it == mCategories.end())
        return false;
    return it->second->containsMatch(matchId);
}

const TatamiList & TournamentStore::getTatamis() const {
    return mTatamis;
}

TatamiList & TournamentStore::getTatamis() {
    return mTatamis;
}

TournamentStore::TournamentStore(const TournamentStore &other)
    : mId(other.mId)
    , mName(other.mName)
    , mTatamis(other.mTatamis)
{
    for (const auto &p : other.mPlayers)
        mPlayers[p.first] = std::make_unique<PlayerStore>(*p.second);
    for (const auto &p : other.mCategories)
        mCategories[p.first] = std::make_unique<CategoryStore>(*p.second);
}

TournamentId TournamentStore::getId() const {
    return mId;
}

void TournamentStore::setId(TournamentId id) {
    mId = id;
}

const std::string & TournamentStore::getWebName() const {
    return mWebName;
}

void TournamentStore::setWebName(const std::string & name) {
    mWebName = name;
}
