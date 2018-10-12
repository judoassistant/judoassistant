#pragma once

#include <QObject>

#include "core.hpp"
#include "serialize.hpp"
#include "stores/tournament_store.hpp"

class QTournamentStore : public QObject, public TournamentStore {
    Q_OBJECT
public:
    QTournamentStore();

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::base_class<TournamentStore>(this));
    }

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
    void beginAddMatches(CategoryId categoryId, std::vector<MatchId> matchIds) override;
    void endAddMatches() override;
    void beginEraseMatches(CategoryId categoryId, std::vector<MatchId> matchIds) override;
    void endEraseMatches() override;
    void beginResetMatches(CategoryId categoryId) override;
    void endResetMatches(CategoryId categoryId) override;

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
};

CEREAL_REGISTER_TYPE(QTournamentStore)
CEREAL_REGISTER_POLYMORPHIC_RELATION(TournamentStore, QTournamentStore)
