#include "stores/qtournament_store.hpp"

QTournamentStore::QTournamentStore()
    : mResettingPlayers(false)
    , mResettingCategories(false)
    , mResettingMatches(false)
{}


void QTournamentStore::changeTournament() {
    log_debug().msg("Emitting tournamentChanged");
    emit tournamentChanged();
}

void QTournamentStore::changePlayers(std::vector<PlayerId> id) {
    log_debug().field("ids", id).msg("Emitting playersChanged");
    emit playersChanged(id);
}

void QTournamentStore::beginAddPlayers(std::vector<PlayerId> id) {
    if (mResettingPlayers)
        return;

    if (mAddedPlayerIds)
        throw std::runtime_error("beginAddPlayers called immidiately after another beginAddPlayers call.");

    log_debug().field("ids", id).msg("Emitting playersAboutToBeAdded");
    emit playersAboutToBeAdded(id);
    mAddedPlayerIds = id;
}

void QTournamentStore::endAddPlayers() {
    if (mResettingPlayers)
        return;

    if (!mAddedPlayerIds)
        throw std::runtime_error("endAddPlayers called with calling beginAddPlayers first.");

    log_debug().field("ids", *mAddedPlayerIds).msg("Emitting playersAdded");
    emit playersAdded(*mAddedPlayerIds);
    mAddedPlayerIds.reset();
}

void QTournamentStore::beginErasePlayers(std::vector<PlayerId> id) {
    if (mResettingPlayers)
        return;

    if (mErasedPlayerIds)
        throw std::runtime_error("beginErasePlayers called immidiately after another beginErasePlayers call.");

    log_debug().field("ids", id).msg("Emitting playersAboutToBeErased");
    emit playersAboutToBeErased(id);
    mErasedPlayerIds = id;

}

void QTournamentStore::endErasePlayers() {
    if (mResettingPlayers)
        return;

    if (!mErasedPlayerIds)
        throw std::runtime_error("endErasePlayers called with calling beginErasePlayers first.");

    log_debug().field("ids", *mErasedPlayerIds).msg("Emitting playersErased");
    emit playersErased(*mErasedPlayerIds);
    mErasedPlayerIds.reset();
}

void QTournamentStore::beginResetPlayers() {
    if (mResettingPlayers)
        throw std::runtime_error("beginResetPlayers called immidiately after another beginResetPlayers call.");

    mResettingPlayers = true;
    log_debug().msg("Emitting playersAboutToBeReset");
    emit playersAboutToBeReset();
}

void QTournamentStore::endResetPlayers() {
    if (!mResettingPlayers)
        throw std::runtime_error("endResetPlayers called with calling beginResetPlayers first.");

    mResettingPlayers = false;
    log_debug().msg("Emitting playersReset");
    emit playersReset();
}

void QTournamentStore::addPlayersToCategory(CategoryId categoryId, std::vector<PlayerId> playerIds) {
    log_debug().field("categoryId", categoryId).field("playerIds", playerIds).msg("Emitting playersAddedToCategory");
    emit playersAddedToCategory(categoryId, playerIds);
}

void QTournamentStore::erasePlayersFromCategory(CategoryId categoryId, std::vector<PlayerId> playerIds) {
    log_debug().field("categoryId", categoryId).field("playerIds", playerIds).msg("Emitting playersErasedFromCategory");
    emit playersErasedFromCategory(categoryId, playerIds);
}

void QTournamentStore::changeCategories(std::vector<CategoryId> id) {
    log_debug().field("ids", id).msg("Emitting categoriesChanged");
    emit categoriesChanged(id);
}

void QTournamentStore::beginAddCategories(std::vector<CategoryId> id) {
    if (mResettingCategories)
        return;

    if (mAddedCategoryIds)
        throw std::runtime_error("beginAddCategories called immidiately after another beginAddCategories call.");

    log_debug().field("ids", id).msg("Emitting categoriesAboutToBeAdded");
    emit categoriesAboutToBeAdded(id);
    mAddedCategoryIds = id;
}

void QTournamentStore::endAddCategories() {
    if (mResettingCategories)
        return;

    if (!mAddedCategoryIds)
        throw std::runtime_error("endAddCategories called with calling beginAddCategories first.");

    log_debug().field("ids", *mAddedCategoryIds).msg("Emitting categoriesAdded");
    emit categoriesAdded(*mAddedCategoryIds);
    mAddedCategoryIds.reset();
}

void QTournamentStore::beginEraseCategories(std::vector<CategoryId> id) {
    if (mResettingCategories)
        return;

    if (mErasedCategoryIds)
        throw std::runtime_error("beginEraseCategories called immidiately after another beginEraseCategories call.");

    log_debug().field("ids", id).msg("Emitting categoriesAboutToBeErased");
    emit categoriesAboutToBeErased(id);
    mErasedCategoryIds = id;
}

void QTournamentStore::endEraseCategories() {
    if (mResettingCategories)
        return;

    if (!mErasedCategoryIds)
        throw std::runtime_error("endEraseCategories called with calling beginEraseCategories first.");

    log_debug().field("ids", *mErasedCategoryIds).msg("Emitting categoriesErased");
    emit categoriesErased(*mErasedCategoryIds);
    mErasedCategoryIds.reset();
}

void QTournamentStore::beginResetCategories() {
    if (mResettingCategories)
        throw std::runtime_error("beginResetCategories called immidiately after another beginResetCategories call.");

    mResettingCategories = true;
    log_debug().msg("Emitting categoriesAboutToBeReset");
    emit categoriesAboutToBeReset();
}

void QTournamentStore::endResetCategories() {
    if (!mResettingCategories)
        throw std::runtime_error("endResetCategories called with calling beginResetCategories first.");

    mResettingCategories = false;
    log_debug().msg("Emitting categoriesReset");
    emit categoriesReset();
}

void QTournamentStore::changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {
    log_debug().field("categoryId", categoryId).field("matchIds", matchIds).msg("Emitting matchesChanged");
    emit matchesChanged(categoryId, matchIds);
}

void QTournamentStore::beginAddMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {
    if (mResettingMatches)
        return;

    if (mAddedMatchIds)
        throw std::runtime_error("beginAddMatches called immidiately after another beginAddMatches call.");

    log_debug().field("categoryId", categoryId).field("matchIds", matchIds).msg("Emitting matchesAboutToBeAdded");
    emit matchesAboutToBeAdded(categoryId, matchIds);
    mAddedMatchIds = matchIds;
    mAddedMatchesCategoryId = categoryId;
}

void QTournamentStore::endAddMatches() {
    if (mResettingMatches)
        return;

    if (!mAddedMatchIds)
        throw std::runtime_error("endAddMatches called with calling beginAddMatches first.");

    log_debug().field("categoryId", *mAddedMatchesCategoryId).field("matchIds", mAddedMatchIds).msg("Emitting matchesAdded");
    emit matchesAdded(*mAddedMatchesCategoryId, *mAddedMatchIds);
    mAddedMatchIds.reset();
    mAddedMatchesCategoryId.reset();
}

void QTournamentStore::beginEraseMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {
    if (mResettingMatches)
        return;

    if (mErasedMatchIds)
        throw std::runtime_error("beginEraseMatches called immidiately after another beginEraseMatches call.");

    log_debug().field("categoryId", categoryId).field("matchIds", matchIds).msg("Emitting matchesAboutToBeErased");
    emit matchesAboutToBeErased(categoryId, matchIds);
    mErasedMatchIds = matchIds;
    mErasedMatchesCategoryId = categoryId;
}

void QTournamentStore::endEraseMatches() {
    if (mResettingMatches)
        return;

    if (!mErasedMatchIds)
        throw std::runtime_error("endEraseMatches called with calling beginEraseMatches first.");

    log_debug().field("categoryId", *mErasedMatchesCategoryId).field("matchIds", *mErasedMatchIds).msg("Emitting matchesErased");
    emit matchesErased(*mErasedMatchesCategoryId, *mErasedMatchIds);
    mErasedMatchIds.reset();
    mErasedMatchesCategoryId.reset();
}

void QTournamentStore::beginResetMatches(CategoryId categoryId) {
    if (mResettingMatches)
        throw std::runtime_error("beginResetMatches called immidiately after another beginResetMatches call.");

    mResettingMatches = true;
    log_debug().field("categoryId", categoryId).msg("Emitting matchesAboutToBeReset");
    emit matchesAboutToBeReset(categoryId);
}

void QTournamentStore::endResetMatches(CategoryId categoryId) {
    if (!mResettingMatches)
        throw std::runtime_error("endResetMatches called with calling beginResetMatches first.");

    mResettingMatches = false;
    log_debug().field("categoryId", categoryId).msg("Emitting matchesReset");
    emit matchesReset(categoryId);
}

