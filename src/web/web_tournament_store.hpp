#pragma once

#include "core/stores/tournament_store.hpp"
#include "web/web_tatami_model.hpp"

class WebTournamentStore : public TournamentStore {
public:
    WebTournamentStore();

    void clearChanges();

    bool tournamentChanged() const;
    const std::unordered_set<PlayerId>& getChangedPlayers() const;
    const std::unordered_set<PlayerId>& getAddedPlayers() const;
    const std::unordered_set<PlayerId>& getErasedPlayers() const;
    const std::unordered_set<PlayerId>& getPlayerMatchResets() const;

    const std::unordered_set<CategoryId>& getChangedCategories() const;
    const std::unordered_set<CategoryId>& getAddedCategories() const;
    const std::unordered_set<CategoryId>& getErasedCategories() const;
    const std::unordered_set<CategoryId>& getCategoryMatchResets() const;
    const std::unordered_set<CategoryId>& getCategoryResultsResets() const;

    const std::unordered_set<CombinedId>& getChangedMatches() const;

    // Parent methods to override
    void changeTournament() override;

    void changePlayers(const std::vector<PlayerId> &playerIds) override;
    void beginAddPlayers(const std::vector<PlayerId> &playerIds) override;
    void endAddPlayers(const std::vector<PlayerId> &playerIds) override;
    void beginErasePlayers(const std::vector<PlayerId> &playerIds) override;
    void endErasePlayers(const std::vector<PlayerId> &playerIds) override;
    void beginResetPlayers() override;
    void endResetPlayers() override;

    void addPlayersToCategory(CategoryId categoryId, const std::vector<PlayerId> &playerIds) override;
    void erasePlayersFromCategory(CategoryId categoryId, const std::vector<PlayerId> &playerIds) override;

    void changeCategories(const std::vector<CategoryId>& categoryIds) override;
    void beginAddCategories(const std::vector<CategoryId>& categoryIds) override;
    void endAddCategories(const std::vector<CategoryId>& categoryIds) override;
    void beginEraseCategories(const std::vector<CategoryId>& categoryIds) override;
    void endEraseCategories(const std::vector<CategoryId>& categoryIds) override;
    void beginResetCategories() override;
    void endResetCategories() override;

    void changeMatches(CategoryId categoryId, const std::vector<MatchId> &matchIds) override;
    void beginResetMatches(const std::vector<CategoryId> &categoryIds) override;
    void endResetMatches(const std::vector<CategoryId> &categoryIds) override;

    void changeTatamis(const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) override;
    void beginAddTatamis(const std::vector<TatamiLocation> &locations) override;
    void endAddTatamis(const std::vector<TatamiLocation> &locations) override;
    void beginEraseTatamis(const std::vector<TatamiLocation> &locations) override;
    void endEraseTatamis(const std::vector<TatamiLocation> &locations) override;

    void addMatchesToPlayer(PlayerId playerId, const std::vector<CombinedId> &matchIds) override;
    void eraseMatchesFromPlayer(PlayerId playerId, const std::vector<CombinedId> &matchIds) override;

    void resetCategoryResults(const std::vector<CategoryId> &categoryId) override;


    void flushWebTatamiModels();
    const WebTatamiModel& getWebTatamiModel(size_t index) const;
    const std::vector<WebTatamiModel>& getWebTatamiModels() const;

private:
    bool mTournamentChanged;

    std::unordered_set<PlayerId> mChangedPlayers;
    std::unordered_set<PlayerId> mAddedPlayers;
    std::unordered_set<PlayerId> mErasedPlayers;
    std::unordered_set<PlayerId> mPlayerMatchResets;

    std::unordered_set<CategoryId> mChangedCategories;
    std::unordered_set<CategoryId> mAddedCategories;
    std::unordered_set<CategoryId> mErasedCategories;
    std::unordered_set<CategoryId> mCategoryMatchResets;
    std::unordered_set<CategoryId> mCategoryResultsResets;

    std::unordered_set<CombinedId> mChangedMatches;

    std::vector<WebTatamiModel> mTatamiModels;
    bool mResettingTatamis;
};

