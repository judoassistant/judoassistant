#include <QColor>
#include <QBrush>
#include <sstream>

#include "core/draw_systems/draw_system.hpp"
#include "core/id.hpp"
#include "core/log.hpp"
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
    auto playerId = pair.first;
    auto position = pair.second;

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


void ResultsModel::changePlayers(const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds) {
        auto it = mPlayers.find(playerId);
        if (it == mPlayers.end())
            continue;
        int row = it->second;
        emit dataChanged(createIndex(row,0), createIndex(row, COLUMN_COUNT-1));
    }
}

void ResultsModel::beginErasePlayers(const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds) {
        if (mPlayers.find(playerId) == mPlayers.end())
            continue;

        beginResetResults();
        return;
    }
}

void ResultsModel::beginResetPlayers() {
    beginResetResults();
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

    mConnections.push(connect(&tournament, &QTournamentStore::categoryResultsReset, this, &ResultsModel::resetCategoryResults));
}

void ResultsModel::beginResetResults() {
    if (mResetting)
        return;
    mResetting = true;
    beginResetModel();
    mPlayers.clear();
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
        auto playerId = mResults[i].first;
        mPlayers[playerId] = i;
    }

    endResetModel();
}

void ResultsModel::resetCategoryResults(CategoryId categoryId) {
    log_debug().msg("Resetting results");
    if (mCategory == categoryId) {
        beginResetResults();
        endResetResults();
    }
}

void ResultsModel::setCategory(std::optional<CategoryId> category) {
    beginResetResults();
    mCategory = category;
    endResetResults();
}

