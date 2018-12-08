#include <QColor>
#include <QBrush>
#include <sstream>

#include "store_managers/store_manager.hpp"
#include "stores/qtournament_store.hpp"
#include "stores/category_store.hpp"
#include "widgets/models/tatami_matches_model.hpp"

TatamiMatchesModel::TatamiMatchesModel(StoreManager &storeManager, size_t tatami, QObject *parent)
    : QAbstractTableModel(parent)
    , mStoreManager(storeManager)
    , mTatami(tatami)
{
    beginResetTournament();
    endResetTournament();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &TatamiMatchesModel::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &TatamiMatchesModel::endResetTournament);
}

void TatamiMatchesModel::beginResetTournament() {
    beginResetModel();

    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    mIds.clear();
    mBlocksLoaded = 0;
}

void TatamiMatchesModel::endResetTournament() {
    auto &tournament = mStoreManager.getTournament();

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisChanged, this, &TatamiMatchesModel::changeTatamis));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesChanged, this, &TatamiMatchesModel::changeMatches));

    auto &tatamis = tournament.getTatamis();
    if (tatamis.tatamiCount() <= mTatami)
        return;
    auto &tatami = tatamis[mTatami];

    while (mIds.size() < ROW_CAP) {
        if (mBlocksLoaded == tatami.groupCount())
            break;

        auto &block = tatami.getGroup(tatami.getHandle(mBlocksLoaded++));

        for (const auto &p : block.getMatches()) {
            auto &category = tournament.getCategory(p.first);
            auto &ruleset = category.getRuleset();
            auto &match = category.getMatch(p.second);

            if (ruleset.isFinished(match)) continue;

            mIds.push_back(p.second);
        }
    }

    endResetModel();
}


int TatamiMatchesModel::rowCount(const QModelIndex &parent) const {
    return mIds.size();
}

int TatamiMatchesModel::columnCount(const QModelIndex &parent) const {
    return static_cast<int>(COLUMN_COUNT);
}

QVariant TatamiMatchesModel::data(const QModelIndex &index, int role) const {
    auto matchId = getMatch(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return QString::fromStdString(matchId.toString());
        }
    }

    if (role == Qt::UserRole) { // Used for sorting
        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

QVariant TatamiMatchesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString(tr("Match Id"));
            }
        }
    }
    return QVariant();
}

std::vector<MatchId> TatamiMatchesModel::getMatches(const QItemSelection &selection) const {
    std::unordered_set<int> rows;
    for (auto index : selection.indexes())
        rows.insert(index.row());

    std::vector<MatchId> matchIds;
    for (auto row : rows)
        matchIds.push_back(getMatch(row));

    return std::move(matchIds);
}

MatchId TatamiMatchesModel::getMatch(int row) const {
    return mIds[row];
}

int TatamiMatchesModel::getRow(MatchId id) const {
    // TODO: Add unordered map for faster lookup (also add to related models)
    size_t row = 0;
    for (MatchId el : mIds) {
        if (el == id)
            break;
        ++row;
    }

    return row;
}

void TatamiMatchesModel::changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {

}

void TatamiMatchesModel::changeTatamis(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {

}

