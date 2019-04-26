#include "core/log.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "core/stores/tatami/tatami_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/player_store.hpp"
#include "core/stores/category_store.hpp"

QTournamentStore::QTournamentStore()
    : QObject()
    , mResettingPlayers(false)
    , mResettingCategories(false)
    , mResettingMatches(false)
{}

QTournamentStore::QTournamentStore(QTournamentStore &&other)
    : QObject()
    , TournamentStore(std::move(other))
{}

QTournamentStore::QTournamentStore(TournamentId id)
    : QObject()
    , TournamentStore(id)
    , mResettingPlayers(false)
    , mResettingCategories(false)
    , mResettingMatches(false)
{}

void QTournamentStore::changeTournament() {
    // log_debug().msg("Emitting tournamentChanged");
    emit tournamentChanged();
}

void QTournamentStore::changePlayers(const std::vector<PlayerId> &playerIds) {
    // log_debug().field("ids", id).msg("Emitting playersChanged");
    emit playersChanged(playerIds);
}

void QTournamentStore::beginAddPlayers(const std::vector<PlayerId> &playerIds) {
    if (mResettingPlayers)
        return;

    emit playersAboutToBeAdded(playerIds);
}

void QTournamentStore::endAddPlayers(const std::vector<PlayerId> &playerIds) {
    if (mResettingPlayers)
        return;

    emit playersAdded(playerIds);
}

void QTournamentStore::beginErasePlayers(const std::vector<PlayerId> &playerIds) {
    if (mResettingPlayers)
        return;

    emit playersAboutToBeErased(playerIds);
}

void QTournamentStore::endErasePlayers(const std::vector<PlayerId> &playerIds) {
    if (mResettingPlayers)
        return;

    emit playersErased(playerIds);
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

void QTournamentStore::addPlayersToCategory(CategoryId categoryId, const std::vector<PlayerId> &playerIds) {
    emit playersAddedToCategory(categoryId, playerIds);
}

void QTournamentStore::erasePlayersFromCategory(CategoryId categoryId, const std::vector<PlayerId> &playerIds) {
    emit playersErasedFromCategory(categoryId, playerIds);
}

void QTournamentStore::changeCategories(const std::vector<CategoryId> &categoryIds) {
    emit categoriesChanged(categoryIds);
}

void QTournamentStore::beginAddCategories(const std::vector<CategoryId> &categoryIds) {
    if (mResettingCategories)
        return;

    emit categoriesAboutToBeAdded(categoryIds);
}

void QTournamentStore::endAddCategories(const std::vector<CategoryId> &categoryIds) {
    if (mResettingCategories)
        return;

    emit categoriesAdded(categoryIds);
}

void QTournamentStore::beginEraseCategories(const std::vector<CategoryId> &categoryIds) {
    if (mResettingCategories)
        return;

    emit categoriesAboutToBeErased(categoryIds);
}

void QTournamentStore::endEraseCategories(const std::vector<CategoryId> &categoryIds) {
    if (mResettingCategories)
        return;

    emit categoriesErased(categoryIds);
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

void QTournamentStore::changeMatches(CategoryId categoryId, const std::vector<MatchId> &matchIds) {
    emit matchesChanged(categoryId, matchIds);
}

void QTournamentStore::beginResetMatches(CategoryId categoryId) {
    if (mResettingMatches)
        throw std::runtime_error("beginResetMatches called immidiately after another beginResetMatches call.");

    mResettingMatches = true;
    emit matchesAboutToBeReset(categoryId);
}

void QTournamentStore::endResetMatches(CategoryId categoryId) {
    if (!mResettingMatches)
        throw std::runtime_error("endResetMatches called with calling beginResetMatches first.");

    mResettingMatches = false;
    emit matchesReset(categoryId);
}

void QTournamentStore::changeTatamis(const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) {
    emit tatamisChanged(locations, blocks);
}

void QTournamentStore::beginAddTatamis(const std::vector<TatamiLocation> &locations) {
    emit tatamisAboutToBeAdded(locations);
}

void QTournamentStore::endAddTatamis(const std::vector<TatamiLocation> &locations) {
    emit tatamisAdded(locations);
}

void QTournamentStore::beginEraseTatamis(const std::vector<TatamiLocation> &locations) {
    emit tatamisAboutToBeErased(locations);
}

void QTournamentStore::endEraseTatamis(const std::vector<TatamiLocation> &locations) {
    emit tatamisErased(locations);
}

void QTournamentStore::resetCategoryResults(CategoryId categoryId) {
    emit categoryResultsReset(categoryId);
}

