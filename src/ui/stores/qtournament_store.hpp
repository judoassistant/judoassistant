#pragma once

#include <QObject>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/actions/action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/stores/tatami/location.hpp"

class QTournamentStore : public QObject, public TournamentStore {
    Q_OBJECT
public:
    QTournamentStore();
    QTournamentStore(TournamentId id);

    // Slightly hacky since QObject does not have a move constructor. Only
    // moves TournamentStore fields
    QTournamentStore(QTournamentStore &&other);

    // signals used for frontends. Called by actions
    void changeTournament() override;

    void changePlayers(std::vector<PlayerId> id) override;
    void beginAddPlayers(std::vector<PlayerId> id) override;
    void endAddPlayers() override;
    void beginErasePlayers(std::vector<PlayerId> id) override;
    void endErasePlayers() override;
    void beginResetPlayers() override;
    void endResetPlayers() override;

    void addPlayersToCategory(CategoryId categoryId, std::vector<PlayerId> playerIds) override;
    void erasePlayersFromCategory(CategoryId categoryId, std::vector<PlayerId> playerIds) override;

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

signals:
    void tournamentChanged();

    void playersChanged(std::vector<PlayerId> id);
    void playersAboutToBeAdded(std::vector<PlayerId> id);
    void playersAdded(std::vector<PlayerId> id);
    void playersAboutToBeErased(std::vector<PlayerId> id);
    void playersErased(std::vector<PlayerId> id);
    void playersAboutToBeReset();
    void playersReset();

    void playersAddedToCategory(CategoryId category, std::vector<PlayerId>);
    void playersErasedFromCategory(CategoryId category, std::vector<PlayerId>);

    void categoriesChanged(std::vector<CategoryId> id);
    void categoriesAboutToBeAdded(std::vector<CategoryId> id);
    void categoriesAdded(std::vector<CategoryId> id);
    void categoriesAboutToBeErased(std::vector<CategoryId> id);
    void categoriesErased(std::vector<CategoryId> id);
    void categoriesAboutToBeReset();
    void categoriesReset();

    void matchesChanged(CategoryId categoryId, std::vector<MatchId> matchIds);
    void matchesAboutToBeReset(CategoryId categoryId);
    void matchesReset(CategoryId categoryId);

    void tatamisChanged(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    void tatamisAboutToBeAdded(std::vector<TatamiLocation> locations);
    void tatamisAdded(std::vector<TatamiLocation> locations);
    void tatamisAboutToBeErased(std::vector<TatamiLocation> locations);
    void tatamisErased(std::vector<TatamiLocation> locations);

private:
    std::optional<std::vector<PlayerId>> mAddedPlayerIds;
    std::optional<std::vector<PlayerId>> mErasedPlayerIds;
    bool mResettingPlayers;

    std::optional<std::vector<CategoryId>> mAddedCategoryIds;
    std::optional<std::vector<CategoryId>> mErasedCategoryIds;
    bool mResettingCategories;

    std::optional<CategoryId> mAddedMatchesCategoryId;
    std::optional<std::vector<MatchId>> mAddedMatchIds;
    std::optional<CategoryId> mErasedMatchesCategoryId;
    std::optional<std::vector<MatchId>> mErasedMatchIds;
    bool mResettingMatches;

    std::optional<std::vector<TatamiLocation>> mAddedTatamiLocations;
    std::optional<std::vector<TatamiLocation>> mErasedTatamiLocations;
};
