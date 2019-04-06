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
    void beginResetMatches(CategoryId categoryId) override;
    void endResetMatches(CategoryId categoryId) override;

    void changeTatamis(const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) override;
    void beginAddTatamis(const std::vector<TatamiLocation> &locations) override;
    void endAddTatamis(const std::vector<TatamiLocation> &locations) override;
    void beginEraseTatamis(const std::vector<TatamiLocation> &locations) override;
    void endEraseTatamis(const std::vector<TatamiLocation> &locations) override;

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
    bool mResettingPlayers;
    bool mResettingCategories;
    bool mResettingMatches;
};

