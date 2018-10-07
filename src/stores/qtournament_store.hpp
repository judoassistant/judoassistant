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

    void changePlayers(std::vector<Id> id) override;
    void beginAddPlayers(std::vector<Id> id) override;
    void endAddPlayers() override;
    void beginErasePlayers(std::vector<Id> id) override;
    void endErasePlayers() override;
    void beginResetPlayers() override;
    void endResetPlayers() override;

    void addPlayersToCategory(Id categoryId, std::vector<Id> playerIds) override;
    void erasePlayersFromCategory(Id categoryId, std::vector<Id> playerIds) override;

    void changeCategories(std::vector<Id> id) override;
    void beginAddCategories(std::vector<Id> id) override;
    void endAddCategories() override;
    void beginEraseCategories(std::vector<Id> id) override;
    void endEraseCategories() override;
    void beginResetCategories() override;
    void endResetCategories() override;

    void changeMatches(std::vector<Id> id) override;
    void beginAddMatches(std::vector<Id> id) override;
    void endAddMatches() override;
    void beginEraseMatches(std::vector<Id> id) override;
    void endEraseMatches() override;
    void beginResetMatches() override;
    void endResetMatches() override;

signals:
    void tournamentChanged();

    void playersChanged(std::vector<Id> id);
    void playersAboutToBeAdded(std::vector<Id> id);
    void playersAdded(std::vector<Id> id);
    void playersAboutToBeErased(std::vector<Id> id);
    void playersErased(std::vector<Id> id);
    void playersAboutToBeReset();
    void playersReset();

    void playersAddedToCategory(Id category, std::vector<Id>);
    void playersErasedFromCategory(Id category, std::vector<Id>);

    void categoriesChanged(std::vector<Id> id);
    void categoriesAboutToBeAdded(std::vector<Id> id);
    void categoriesAdded(std::vector<Id> id);
    void categoriesAboutToBeErased(std::vector<Id> id);
    void categoriesErased(std::vector<Id> id);
    void categoriesAboutToBeReset();
    void categoriesReset();

    void matchesChanged(std::vector<Id> id);
    void matchesAboutToBeAdded(std::vector<Id> id);
    void matchesAdded(std::vector<Id> id);
    void matchesAboutToBeErased(std::vector<Id> id);
    void matchesErased(std::vector<Id> id);
    void matchesAboutToBeReset();
    void matchesReset();
private:
    std::optional<std::vector<Id>> mAddedPlayerIds;
    std::optional<std::vector<Id>> mErasedPlayerIds;
    bool mResettingPlayers;

    std::optional<std::vector<Id>> mAddedCategoryIds;
    std::optional<std::vector<Id>> mErasedCategoryIds;
    bool mResettingCategories;

    std::optional<std::vector<Id>> mAddedMatchIds;
    std::optional<std::vector<Id>> mErasedMatchIds;
    bool mResettingMatches;
};

CEREAL_REGISTER_TYPE(QTournamentStore)
CEREAL_REGISTER_POLYMORPHIC_RELATION(TournamentStore, QTournamentStore)
