#pragma once

#include <QObject>

#include "core.hpp"
#include "id.hpp"
#include "actions/action.hpp"
#include "draw_systems/draw_system.hpp"
#include "rulesets/ruleset.hpp"
#include "stores/match_event.hpp"
#include "stores/match_store.hpp"
#include "stores/tournament_store.hpp"

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

    void changeTatamis(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) override;
    void beginAddTatamis(std::vector<size_t> ids) override;
    void endAddTatamis() override;
    void beginEraseTatamis(std::vector<size_t> ids) override;
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
    void matchesAboutToBeAdded(CategoryId categoryId, std::vector<MatchId> matchIds);
    void matchesAdded(CategoryId categoryId, std::vector<MatchId> matchIds);
    void matchesAboutToBeErased(CategoryId categoryId, std::vector<MatchId> matchIds);
    void matchesErased(CategoryId categoryId, std::vector<MatchId> matchIds);
    void matchesAboutToBeReset(CategoryId categoryId);
    void matchesReset(CategoryId categoryId);

    void tatamisChanged(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    void tatamisAboutToBeAdded(std::vector<size_t> id);
    void tatamisAdded(std::vector<size_t> id);
    void tatamisAboutToBeErased(std::vector<size_t> id);
    void tatamisErased(std::vector<size_t> id);

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

    std::optional<std::vector<size_t>> mAddedTatamiIds;
    std::optional<std::vector<size_t>> mErasedTatamiIds;
};

