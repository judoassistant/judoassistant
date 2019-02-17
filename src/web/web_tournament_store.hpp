#pragma once

#include "core/stores/tournament_store.hpp"

class WebTournamentStore : public TournamentStore {
public:
    WebTournamentStore();

    void clearChanges();

    bool tournamentChanged() const;
    const std::unordered_set<PlayerId>& getChangedPlayers() const;
    const std::unordered_set<PlayerId>& getAddedPlayers() const;
    const std::unordered_set<PlayerId>& getErasedPlayers() const;

    const std::unordered_set<CategoryId>& getChangedCategories() const;
    const std::unordered_set<CategoryId>& getAddedCategories() const;
    const std::unordered_set<CategoryId>& getErasedCategories() const;

    const std::unordered_set<std::pair<CategoryId, MatchId>>& getChangedMatches() const;
    const std::unordered_set<std::pair<CategoryId, MatchId>>& getAddedMatches() const;
    const std::unordered_set<std::pair<CategoryId, MatchId>>& getErasedMatches() const;

    // Parent methods to override
    void changeTournament() override;

    void changePlayers(std::vector<PlayerId> ids) override;
    void beginAddPlayers(std::vector<PlayerId> ids) override;
    void endAddPlayers() override;
    void beginErasePlayers(std::vector<PlayerId> ids) override;
    void endErasePlayers() override;
    void beginResetPlayers() override;
    void endResetPlayers() override;

    void addPlayersToCategory(CategoryId category, std::vector<PlayerId> ids) override;
    void erasePlayersFromCategory(CategoryId category, std::vector<PlayerId> ids) override;

    void changeCategories(std::vector<CategoryId> id) override;
    void beginAddCategories(std::vector<CategoryId> id) override;
    void endAddCategories() override;
    void beginEraseCategories(std::vector<CategoryId> id) override;
    void endEraseCategories() override;
    void beginResetCategories() override;
    void endResetCategories() override;

    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) override;
    void beginResetMatches(CategoryId categoryId) override;
    void endResetMatches(CategoryId categoryId) override;

    void changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) override;
    void beginAddTatamis(std::vector<TatamiLocation> locations) override;
    void endAddTatamis() override;
    void beginEraseTatamis(std::vector<TatamiLocation> locations) override;
    void endEraseTatamis() override;

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

    std::unordered_set<std::pair<CategoryId, MatchId>> mChangedMatches;
    std::unordered_set<std::pair<CategoryId, MatchId>> mAddedMatches;
    std::unordered_set<std::pair<CategoryId, MatchId>> mErasedMatches;
};

