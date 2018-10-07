#include "stores/qtournament_store.hpp"

QTournamentStore::QTournamentStore()
    : mResettingPlayers(false)
    , mResettingCategories(false)
    , mResettingMatches(false)
{}


void QTournamentStore::changeTournament() {
    emit tournamentChanged();
}

void QTournamentStore::changePlayers(std::vector<Id> id) {
    emit playersChanged(id);
}

void QTournamentStore::beginAddPlayers(std::vector<Id> id) {
    if (mResettingPlayers)
        return;

    if (mAddedPlayerIds)
        throw std::runtime_error("beginAddPlayers called immidiately after another beginAddPlayers call.");

    emit playersAboutToBeAdded(id);
    mAddedPlayerIds = id;
}

void QTournamentStore::endAddPlayers() {
    if (mResettingPlayers)
        return;

    if (!mAddedPlayerIds)
        throw std::runtime_error("endAddPlayers called with calling beginAddPlayers first.");

    emit playersAdded(*mAddedPlayerIds);
    mAddedPlayerIds.reset();
}

void QTournamentStore::beginErasePlayers(std::vector<Id> id) {
    if (mResettingPlayers)
        return;

    if (mErasedPlayerIds)
        throw std::runtime_error("beginErasePlayers called immidiately after another beginErasePlayers call.");

    emit playersAboutToBeErased(id);
    mErasedPlayerIds = id;

}

void QTournamentStore::endErasePlayers() {
    if (mResettingPlayers)
        return;

    if (!mErasedPlayerIds)
        throw std::runtime_error("endErasePlayers called with calling beginErasePlayers first.");

    emit playersErased(*mErasedPlayerIds);
    mErasedPlayerIds.reset();
}

void QTournamentStore::beginResetPlayers() {
    if (mResettingPlayers)
        throw std::runtime_error("beginResetPlayers called immidiately after another beginResetPlayers call.");

    mResettingPlayers = true;
    emit playersAboutToBeReset();
}

void QTournamentStore::endResetPlayers() {
    if (!mResettingPlayers)
        throw std::runtime_error("endResetPlayers called with calling beginResetPlayers first.");

    mResettingPlayers = false;
    emit playersReset();
}

void QTournamentStore::addPlayersToCategory(Id categoryId, std::vector<Id> playerIds) {
    emit playersAddedToCategory(categoryId, playerIds);
}

void QTournamentStore::erasePlayersFromCategory(Id categoryId, std::vector<Id> playerIds) {
    emit playersErasedFromCategory(categoryId, playerIds);
}

void QTournamentStore::changeCategories(std::vector<Id> id) {
    emit categoriesChanged(id);
}

void QTournamentStore::beginAddCategories(std::vector<Id> id) {
    if (mResettingCategories)
        return;

    if (mAddedCategoryIds)
        throw std::runtime_error("beginAddCategories called immidiately after another beginAddCategories call.");

    emit categoriesAboutToBeAdded(id);
    mAddedCategoryIds = id;
}

void QTournamentStore::endAddCategories() {
    if (mResettingCategories)
        return;

    if (!mAddedCategoryIds)
        throw std::runtime_error("endAddCategories called with calling beginAddCategories first.");

    emit categoriesAdded(*mAddedCategoryIds);
    mAddedCategoryIds.reset();
}

void QTournamentStore::beginEraseCategories(std::vector<Id> id) {
    if (mResettingCategories)
        return;

    if (mErasedCategoryIds)
        throw std::runtime_error("beginEraseCategories called immidiately after another beginEraseCategories call.");

    emit categoriesAboutToBeErased(id);
    mErasedCategoryIds = id;
}

void QTournamentStore::endEraseCategories() {
    if (mResettingCategories)
        return;

    if (!mErasedCategoryIds)
        throw std::runtime_error("endEraseCategories called with calling beginEraseCategories first.");

    emit categoriesErased(*mErasedCategoryIds);
    mErasedCategoryIds.reset();
}

void QTournamentStore::beginResetCategories() {
    if (mResettingCategories)
        throw std::runtime_error("beginResetCategories called immidiately after another beginResetCategories call.");

    mResettingCategories = true;
    emit categoriesAboutToBeReset();
}

void QTournamentStore::endResetCategories() {
    if (!mResettingCategories)
        throw std::runtime_error("endResetCategories called with calling beginResetCategories first.");

    mResettingCategories = false;
    emit categoriesReset();
}

void QTournamentStore::changeMatches(std::vector<Id> id) {
    emit matchesChanged(id);
}

void QTournamentStore::beginAddMatches(std::vector<Id> id) {
    if (mResettingMatches)
        return;

    if (mAddedMatchIds)
        throw std::runtime_error("beginAddMatches called immidiately after another beginAddMatches call.");

    emit matchesAboutToBeAdded(id);
    mAddedMatchIds = id;
}

void QTournamentStore::endAddMatches() {
    if (mResettingMatches)
        return;

    if (!mAddedMatchIds)
        throw std::runtime_error("endAddMatches called with calling beginAddMatches first.");

    emit matchesAdded(*mAddedMatchIds);
    mAddedMatchIds.reset();
}

void QTournamentStore::beginEraseMatches(std::vector<Id> id) {
    if (mResettingMatches)
        return;

    if (mErasedMatchIds)
        throw std::runtime_error("beginEraseMatches called immidiately after another beginEraseMatches call.");

    emit matchesAboutToBeErased(id);
    mErasedMatchIds = id;
}

void QTournamentStore::endEraseMatches() {
    if (mResettingMatches)
        return;

    if (!mErasedMatchIds)
        throw std::runtime_error("endEraseMatches called with calling beginEraseMatches first.");

    emit matchesErased(*mErasedMatchIds);
    mErasedMatchIds.reset();
}

void QTournamentStore::beginResetMatches() {
    if (mResettingMatches)
        throw std::runtime_error("beginResetMatches called immidiately after another beginResetMatches call.");

    mResettingMatches = true;
    emit matchesAboutToBeReset();
}

void QTournamentStore::endResetMatches() {
    if (!mResettingMatches)
        throw std::runtime_error("endResetMatches called with calling beginResetMatches first.");

    mResettingMatches = false;
    emit matchesReset();
}
