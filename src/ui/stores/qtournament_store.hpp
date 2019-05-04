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
    // QTournamentStore(TournamentId id);

    // Slightly hacky since QObject does not have a move constructor. Only
    // moves TournamentStore fields
    QTournamentStore(QTournamentStore &&other);

    // signals used for frontends. Called by actions
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

    void resetCategoryResults(const std::vector<CategoryId> &categoryIds) override;

    void changePreferences() override;

signals:
    void tournamentChanged();

    void playersChanged(const std::vector<PlayerId> &playerIds);
    void playersAboutToBeAdded(const std::vector<PlayerId> &playerIds);
    void playersAdded(const std::vector<PlayerId> &playerIds);
    void playersAboutToBeErased(const std::vector<PlayerId> &playerIds);
    void playersErased(const std::vector<PlayerId> &playerIds);
    void playersAboutToBeReset();
    void playersReset();

    void playersAddedToCategory(CategoryId categoryId, const std::vector<PlayerId> &playerIds);
    void playersErasedFromCategory(CategoryId categoryId, const std::vector<PlayerId> &playerIds);

    void categoriesChanged(const std::vector<CategoryId> &categoryIds);
    void categoriesAboutToBeAdded(const std::vector<CategoryId> &categoryIds);
    void categoriesAdded(const std::vector<CategoryId> &categoryIds);
    void categoriesAboutToBeErased(const std::vector<CategoryId> &categoryIds);
    void categoriesErased(const std::vector<CategoryId> &categoryIds);
    void categoriesAboutToBeReset();
    void categoriesReset();

    void matchesChanged(CategoryId categoryId, const std::vector<MatchId> &matchIds);
    void matchesAboutToBeReset(const std::vector<CategoryId> &categoryId);
    void matchesReset(const std::vector<CategoryId> &categoryId);

    void tatamisChanged(const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks);
    void tatamisAboutToBeAdded(const std::vector<TatamiLocation> &locations);
    void tatamisAdded(const std::vector<TatamiLocation> &locations);
    void tatamisAboutToBeErased(const std::vector<TatamiLocation> &locations);
    void tatamisErased(const std::vector<TatamiLocation> &locations);

    void categoryResultsReset(const std::vector<CategoryId> &categoryId);

    void preferencesChanged();
private:
    bool mResettingPlayers;
    bool mResettingCategories;
    bool mResettingMatches;
};

