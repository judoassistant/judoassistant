#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "web/web_tournament_store.hpp"

// TODO: Handle changes to match players
WebTournamentStore::WebTournamentStore()
    : mTournamentChanged(false)
{}

void WebTournamentStore::clearChanges() {
    mTournamentChanged = false;
    mChangedPlayers.clear();
    mAddedPlayers.clear();
    mErasedPlayers.clear();

    mChangedCategories.clear();
    mAddedCategories.clear();
    mErasedCategories.clear();

    mChangedMatches.clear();
    mAddedMatches.clear();
    mErasedMatches.clear();
}

void WebTournamentStore::changeTournament() {
    mTournamentChanged = true;
}

void WebTournamentStore::changePlayers(std::vector<PlayerId> ids) {
    for (auto id : ids) {
        assert(mErasedPlayers.find(id) == mErasedPlayers.end());
        if (mAddedPlayers.find(id) != mAddedPlayers.end())
            continue;
        mChangedPlayers.insert(id);
    }
}

void WebTournamentStore::beginAddPlayers(std::vector<PlayerId> ids) {
    for (auto id : ids) {
        assert(mChangedPlayers.find(id) == mChangedPlayers.end());
        mPlayerMatchResets.insert(id);
        if (mErasedPlayers.find(id) != mErasedPlayers.end()) {
            mErasedPlayers.erase(id);
            mChangedPlayers.insert(id);
        }
        else {
            mAddedPlayers.insert(id);
        }
    }
}

void WebTournamentStore::endAddPlayers() {
    // noop
}

void WebTournamentStore::beginErasePlayers(std::vector<PlayerId> ids) {
    for (auto id : ids) {
        assert(mErasedPlayers.find(id) == mErasedPlayers.end());
        mPlayerMatchResets.erase(id);
        if (mAddedPlayers.find(id) != mAddedPlayers.end()) {
            mAddedPlayers.erase(id);
        }
        else {
            mChangedPlayers.erase(id);
            mErasedPlayers.insert(id);
        }
    }
}

void WebTournamentStore::endErasePlayers() {
    // noop
}

void WebTournamentStore::beginResetPlayers() {
    // Erase all players
    std::vector<PlayerId> ids;
    for (const auto &p : getPlayers())
        ids.push_back(p.first);

    beginErasePlayers(ids);
}

void WebTournamentStore::endResetPlayers() {
    // Add all players
    std::vector<PlayerId> ids;
    for (const auto &p : getPlayers())
        ids.push_back(p.first);

    beginAddPlayers(ids);
}

void WebTournamentStore::addPlayersToCategory(CategoryId category, std::vector<PlayerId> ids) {
    for (auto id : ids)
        mPlayerMatchResets.insert(id);
    mCategoryMatchResets.insert(category);
}

void WebTournamentStore::erasePlayersFromCategory(CategoryId category, std::vector<PlayerId> ids) {
    for (auto id : ids)
        mPlayerMatchResets.insert(id);
    mCategoryMatchResets.insert(category);
}

void WebTournamentStore::changeCategories(std::vector<CategoryId> ids) {
    for (auto id : ids) {
        assert(mErasedCategories.find(id) == mErasedCategories.end());
        if (mAddedCategories.find(id) != mAddedCategories.end())
            continue;
        mChangedCategories.insert(id);
    }
}

void WebTournamentStore::beginAddCategories(std::vector<CategoryId> ids) {
    for (auto id : ids) {
        assert(mChangedCategories.find(id) == mChangedCategories.end());
        mCategoryMatchResets.insert(id);
        if (mErasedCategories.find(id) != mErasedCategories.end()) {
            mErasedCategories.erase(id);
            mChangedCategories.insert(id);
        }
        else {
            mAddedCategories.insert(id);
        }
    }
}

void WebTournamentStore::endAddCategories() {
    // noop
}

void WebTournamentStore::beginEraseCategories(std::vector<CategoryId> ids) {
    for (auto id : ids) {
        assert(mErasedCategories.find(id) == mErasedCategories.end());
        mCategoryMatchResets.erase(id);
        if (mAddedCategories.find(id) != mAddedCategories.end()) {
            mAddedCategories.erase(id);
        }
        else {
            mChangedCategories.erase(id);
            mErasedCategories.insert(id);
        }
    }
}

void WebTournamentStore::endEraseCategories() {
    // noop
}

void WebTournamentStore::beginResetCategories() {
    // Erase all categories
    std::vector<CategoryId> ids;
    for (const auto &p : getCategories())
        ids.push_back(p.first);

    beginEraseCategories(ids);
}

void WebTournamentStore::endResetCategories() {
    // Add all categories
    std::vector<CategoryId> ids;
    for (const auto &p : getCategories())
        ids.push_back(p.first);

    beginAddCategories(ids);
}

void WebTournamentStore::changeMatches(CategoryId category, std::vector<MatchId> ids) {
    for (auto id : ids) {
        auto combinedId = std::make_pair(category, id);
        assert(mErasedMatches.find(combinedId) == mErasedMatches.end());
        if (mAddedMatches.find(combinedId) != mAddedMatches.end())
            continue;
        mChangedMatches.insert(combinedId);
    }
}

void WebTournamentStore::beginResetMatches(CategoryId category) {
    // Erase all matches
    mCategoryMatchResets.insert(category);
    for (auto player : getCategory(category).getPlayers())
        mPlayerMatchResets.insert(player);

    for (const auto &match : getCategory(category).getMatches()) {
        auto combinedId = std::make_pair(category, match->getId());
        assert(mChangedMatches.find(combinedId) == mChangedMatches.end());
        if (mErasedMatches.find(combinedId) != mErasedMatches.end()) {
            mErasedMatches.erase(combinedId);
            mChangedMatches.insert(combinedId);
        }
        else {
            mAddedMatches.insert(combinedId);
        }
    }
}

void WebTournamentStore::endResetMatches(CategoryId category) {
    // Add all matches
    for (const auto &match : getCategory(category).getMatches()) {
        auto combinedId = std::make_pair(category, match->getId());
        assert(mErasedMatches.find(combinedId) == mErasedMatches.end());
        if (mAddedMatches.find(combinedId) != mAddedMatches.end()) {
            mAddedMatches.erase(combinedId);
        }
        else {
            mChangedMatches.erase(combinedId);
            mErasedMatches.insert(combinedId);
        }
    }
}

void WebTournamentStore::changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {
    // TODO: Implement
}

void WebTournamentStore::beginAddTatamis(std::vector<TatamiLocation> locations) {
    // TODO: Implement
}

void WebTournamentStore::endAddTatamis() {
    // TODO: Implement
}

void WebTournamentStore::beginEraseTatamis(std::vector<TatamiLocation> locations) {
    // TODO: Implement
}

void WebTournamentStore::endEraseTatamis() {
    // TODO: Implement
}

