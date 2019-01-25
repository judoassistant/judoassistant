#include <QColor>
#include <QBrush>
#include <sstream>

#include "core/draw_systems/draw_system.hpp"
#include "core/id.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "ui/models/results_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"

ResultsModel::ResultsModel(StoreManager & storeManager, QObject * parent)
    : QAbstractTableModel(parent)
    , mStoreManager(storeManager)
{
    beginResetTournament();
    endResetTournament();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &ResultsModel::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &ResultsModel::endResetTournament);
}

int ResultsModel::rowCount(const QModelIndex &parent) const {
    return mResults.size();
}

int ResultsModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant ResultsModel::data(const QModelIndex &index, int role) const {
    auto pair = mResults[index.row()];
    auto position = pair.first;
    auto playerId = pair.second;

    const PlayerStore &player = mStoreManager.getTournament().getPlayer(playerId);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return QString::fromStdString(player.getFirstName());
            case 1:
                return QString::fromStdString(player.getLastName());
            case 2:
                return (position ? QVariant(*position) : QVariant(""));
        }
    }

    return QVariant();
}

QVariant ResultsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString(tr("First Name"));
                case 1:
                    return QString(tr("Last Name"));
                case 2:
                    return QString(tr("Position"));
            }
        }
    }
    return QVariant();
}


void ResultsModel::changePlayers(std::vector<PlayerId> ids) {
    for (auto id : ids) {
        auto it = mPlayers.find(id);
        if (it == mPlayers.end())
            continue;
        int row = it->second;
        emit dataChanged(createIndex(row,0), createIndex(row, COLUMN_COUNT-1));
    }
}

void ResultsModel::beginErasePlayers(std::vector<PlayerId> ids) {
    for (auto id : ids) {
        if (mPlayers.find(id) == mPlayers.end())
            continue;

        beginResetResults();
        return;
    }
}

void ResultsModel::beginResetPlayers() {
    beginResetResults();
}

void ResultsModel::beginResetMatches(CategoryId categoryId) {
    beginResetResults();
}

void ResultsModel::endResetMatches(CategoryId categoryId) {
    endResetResults();
}

void ResultsModel::beginResetTournament() {
    beginResetResults();

    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void ResultsModel::endResetTournament() {
    endResetResults();

    auto &tournament = mStoreManager.getTournament();

    mConnections.push(connect(&tournament, &QTournamentStore::playersChanged, this, &ResultsModel::changePlayers));
    mConnections.push(connect(&tournament, &QTournamentStore::playersAboutToBeErased, this, &ResultsModel::beginErasePlayers));
    mConnections.push(connect(&tournament, &QTournamentStore::playersAboutToBeReset, this, &ResultsModel::beginResetPlayers));

    mConnections.push(connect(&tournament, &QTournamentStore::matchesChanged, this, &ResultsModel::changeMatches));

    mConnections.push(connect(&tournament, &QTournamentStore::matchesAboutToBeReset, this, &ResultsModel::beginResetMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesReset, this, &ResultsModel::endResetMatches));
}

void ResultsModel::beginResetResults() {
    if (mResetting)
        return;
    mResetting = true;
    beginResetModel();
    mPlayers.clear();
    mMatches.clear();
    mResults.clear();
}

void ResultsModel::endResetResults() {
    assert(mResetting);
    mResetting = false;

    if (!mCategory)
        return;

    const auto &tournament = mStoreManager.getTournament();
    const auto &category = tournament.getCategory(*mCategory);

    mResults = category.getDrawSystem().getResults(tournament, category);
    for (size_t i = 0; i < mResults.size(); ++i) {
        auto playerId = mResults[i].second;
        mPlayers[playerId] = i;
    }

    for (const auto &matchPtr : category.getMatches())
        mMatches[matchPtr->getId()] = matchPtr->getStatus();

    endResetModel();
}

void ResultsModel::changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {
    if (mCategory != categoryId)
        return;

    bool shouldReset = false;
    const auto &category = mStoreManager.getTournament().getCategory(*mCategory);
    for (auto matchId : matchIds) {
        const auto &match = category.getMatch(matchId);
        auto it = mMatches.find(matchId);
        assert(it != mMatches.end());

        // The results only change when matches are finished or were finished
        if (match.getStatus() == MatchStatus::FINISHED || it->second == MatchStatus::FINISHED)
            shouldReset = true;
        it->second = match.getStatus();
    }

    if (shouldReset) {
        beginResetResults();
        endResetResults();
    }
}

void ResultsModel::setCategory(std::optional<CategoryId> category) {
    beginResetResults();
    mCategory = category;
    endResetResults();
}

