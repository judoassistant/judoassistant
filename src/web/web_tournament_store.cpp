#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "web/web_tournament_store.hpp"

WebTournamentStore::WebTournamentStore()
    : mTournamentChanged(false)
    , mResettingTatamis(true)
{}

void WebTournamentStore::clearChanges() {
    mTournamentChanged = false;
    mChangedPlayers.clear();
    mAddedPlayers.clear();
    mErasedPlayers.clear();
    mPlayerMatchResets.clear();

    mChangedCategories.clear();
    mAddedCategories.clear();
    mErasedCategories.clear();
    mCategoryMatchResets.clear();

    mChangedMatches.clear();

    mResettingTatamis = false;
}

void WebTournamentStore::changeTournament() {
    mTournamentChanged = true;
}

void WebTournamentStore::changePlayers(const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds) {
        assert(mErasedPlayers.find(playerId) == mErasedPlayers.end());
        if (mAddedPlayers.find(playerId) != mAddedPlayers.end())
            continue;
        mChangedPlayers.insert(playerId);
    }
}

void WebTournamentStore::beginAddPlayers(const std::vector<PlayerId> &playerIds) {
    // noop
}

void WebTournamentStore::endAddPlayers(const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds) {
        assert(mChangedPlayers.find(playerId) == mChangedPlayers.end());
        mPlayerMatchResets.insert(playerId);
        if (mErasedPlayers.find(playerId) != mErasedPlayers.end()) {
            mErasedPlayers.erase(playerId);
            mChangedPlayers.insert(playerId);
        }
        else {
            mAddedPlayers.insert(playerId);
        }
    }
}

void WebTournamentStore::beginErasePlayers(const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds) {
        assert(mErasedPlayers.find(playerId) == mErasedPlayers.end());
        mPlayerMatchResets.erase(playerId);
        if (mAddedPlayers.find(playerId) != mAddedPlayers.end()) {
            mAddedPlayers.erase(playerId);
        }
        else {
            mChangedPlayers.erase(playerId);
            mErasedPlayers.insert(playerId);
        }
    }
}

void WebTournamentStore::endErasePlayers(const std::vector<PlayerId> &playerIds) {
    // noop
}

void WebTournamentStore::beginResetPlayers() {
    // Erase all players
    std::vector<PlayerId> playerIds;
    for (const auto &p : getPlayers())
        playerIds.push_back(p.first);

    beginErasePlayers(playerIds);
}

void WebTournamentStore::endResetPlayers() {
    // Add all players
    std::vector<PlayerId> playerIds;
    for (const auto &p : getPlayers())
        playerIds.push_back(p.first);

    beginAddPlayers(playerIds);
}

void WebTournamentStore::addPlayersToCategory(CategoryId categoryId, const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds)
        mPlayerMatchResets.insert(playerId);
    mCategoryMatchResets.insert(categoryId);
}

void WebTournamentStore::erasePlayersFromCategory(CategoryId categoryId, const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds)
        mPlayerMatchResets.insert(playerId);
    mCategoryMatchResets.insert(categoryId);
}

void WebTournamentStore::changeCategories(const std::vector<CategoryId> &categoryIds) {
    for (auto categoryId : categoryIds) {
        assert(mErasedCategories.find(categoryId) == mErasedCategories.end());
        if (mAddedCategories.find(categoryId) != mAddedCategories.end())
            continue;
        mChangedCategories.insert(categoryId);
    }
}

void WebTournamentStore::beginAddCategories(const std::vector<CategoryId> &categoryIds) {
    // noop
}

void WebTournamentStore::endAddCategories(const std::vector<CategoryId> &categoryIds) {
    for (auto categoryId : categoryIds) {
        assert(mChangedCategories.find(categoryId) == mChangedCategories.end());
        mCategoryMatchResets.insert(categoryId);
        if (mErasedCategories.find(categoryId) != mErasedCategories.end()) {
            mErasedCategories.erase(categoryId);
            mChangedCategories.insert(categoryId);
        }
        else {
            mAddedCategories.insert(categoryId);
        }

        for (auto playerId : getCategory(categoryId).getPlayers())
            mPlayerMatchResets.insert(playerId);
    }
}

void WebTournamentStore::beginEraseCategories(const std::vector<CategoryId>& categoryIds) {
    for (auto categoryId : categoryIds) {
        assert(mErasedCategories.find(categoryId) == mErasedCategories.end());
        mCategoryMatchResets.erase(categoryId);
        if (mAddedCategories.find(categoryId) != mAddedCategories.end()) {
            mAddedCategories.erase(categoryId);
        }
        else {
            mChangedCategories.erase(categoryId);
            mErasedCategories.insert(categoryId);
        }

        for (auto playerId : getCategory(categoryId).getPlayers())
            mPlayerMatchResets.insert(playerId);
    }
}

void WebTournamentStore::endEraseCategories(const std::vector<CategoryId>& categoryIds) {
    // noop
}

void WebTournamentStore::beginResetCategories() {
    // Erase all categories
    std::vector<CategoryId> categoryIds;
    for (const auto &p : getCategories())
        categoryIds.push_back(p.first);

    beginEraseCategories(categoryIds);
}

void WebTournamentStore::endResetCategories() {
    // Add all categories
    std::vector<CategoryId> categoryIds;
    for (const auto &p : getCategories())
        categoryIds.push_back(p.first);

    endAddCategories(categoryIds);
}

void WebTournamentStore::changeMatches(CategoryId categoryId, const std::vector<MatchId> &matchIds) {
    for (auto matchId : matchIds) {
        auto combinedId = std::make_pair(categoryId, matchId);
        mChangedMatches.insert(combinedId);
    }

    if (!mResettingTatamis) {
        for (WebTatamiModel &model : mTatamiModels)
            model.changeMatches(*this, categoryId, matchIds);
    }
}

void WebTournamentStore::beginResetMatches(CategoryId categoryId) {
    // noop

    // for (const auto &match : getCategory(category).getMatches()) {
    //     auto combinedId = std::make_pair(category, match->getId());
    //     assert(mChangedMatches.find(combinedId) == mChangedMatches.end());
    //     if (mErasedMatches.find(combinedId) != mErasedMatches.end()) {
    //         mErasedMatches.erase(combinedId);
    //         mChangedMatches.insert(combinedId);
    //     }
    //     else {
    //         mAddedMatches.insert(combinedId);
    //     }
    // }
}

void WebTournamentStore::endResetMatches(CategoryId categoryId) {
    mCategoryMatchResets.insert(categoryId);
    for (auto playerId : getCategory(categoryId).getPlayers())
        mPlayerMatchResets.insert(playerId);

    // Add all matches
    // for (const auto &match : getCategory(category).getMatches()) {
    //     auto combinedId = std::make_pair(category, match->getId());
    //     assert(mErasedMatches.find(combinedId) == mErasedMatches.end());
    //     if (mAddedMatches.find(combinedId) != mAddedMatches.end()) {
    //         mAddedMatches.erase(combinedId);
    //     }
    //     else {
    //         mChangedMatches.erase(combinedId);
    //         mErasedMatches.insert(combinedId);
    //     }
    // }
}

void WebTournamentStore::changeTatamis(const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) {
    if (!mResettingTatamis) {
        for (WebTatamiModel &model : mTatamiModels)
            model.changeTatamis(*this, locations, blocks);
    }
}

void WebTournamentStore::beginAddTatamis(const std::vector<TatamiLocation> &locations) {
    mResettingTatamis = true;
}

void WebTournamentStore::endAddTatamis(const std::vector<TatamiLocation> &locations) {
    mResettingTatamis = true;
}

void WebTournamentStore::beginEraseTatamis(const std::vector<TatamiLocation> &locations) {
    mResettingTatamis = true;
}

void WebTournamentStore::endEraseTatamis(const std::vector<TatamiLocation> &locations) {
    mResettingTatamis = true;
}

bool WebTournamentStore::tournamentChanged() const {
    return mTournamentChanged;
}

const std::unordered_set<PlayerId>& WebTournamentStore::getChangedPlayers() const {
    return mChangedPlayers;
}

const std::unordered_set<PlayerId>& WebTournamentStore::getAddedPlayers() const {
    return mAddedPlayers;
}

const std::unordered_set<PlayerId>& WebTournamentStore::getErasedPlayers() const {
    return mErasedPlayers;
}

const std::unordered_set<PlayerId>& WebTournamentStore::getPlayerMatchResets() const {
    return mPlayerMatchResets;
}

const std::unordered_set<CategoryId>& WebTournamentStore::getChangedCategories() const {
    return mChangedCategories;
}

const std::unordered_set<CategoryId>& WebTournamentStore::getAddedCategories() const {
    return mAddedCategories;
}

const std::unordered_set<CategoryId>& WebTournamentStore::getErasedCategories() const {
    return mErasedCategories;
}

const std::unordered_set<CategoryId>& WebTournamentStore::getCategoryMatchResets() const {
    return mCategoryMatchResets;
}

const std::unordered_set<std::pair<CategoryId, MatchId>>& WebTournamentStore::getChangedMatches() const {
    return mChangedMatches;
}

void WebTournamentStore::flushWebTatamiModels() {
    if (!mResettingTatamis)
        return;

    mTatamiModels.clear();
    for (size_t i = 0; i < getTatamis().tatamiCount(); ++i)
        mTatamiModels.emplace_back(*this, i);
    mResettingTatamis = false;
}

const WebTatamiModel& WebTournamentStore::getWebTatamiModel(size_t index) const {
    assert(!mResettingTatamis);
    return mTatamiModels.at(index);
}

