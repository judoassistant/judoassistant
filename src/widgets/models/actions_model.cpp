#include <QColor>
#include <QBrush>
#include <sstream>

#include "actions/action.hpp"
#include "store_managers/store_manager.hpp"
#include "stores/qtournament_store.hpp"
#include "stores/category_store.hpp"
#include "stores/match_store.hpp"
#include "widgets/models/actions_model.hpp"

ActionsModel::ActionsModel(StoreManager & storeManager, QObject * parent)
    : QAbstractTableModel(parent)
    , mStoreManager(storeManager)
{
    aboutToReset();
    reset();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &ActionsModel::aboutToReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &ActionsModel::reset);
    connect(&mStoreManager, &StoreManager::actionAboutToBeErased, this, &ActionsModel::eraseAction);
    connect(&mStoreManager, &StoreManager::actionAdded, this, &ActionsModel::addAction);
}

void ActionsModel::aboutToReset() {
    beginResetModel();
    mIds.clear();
}

void ActionsModel::reset() {
    for (auto it = mStoreManager.actionsBegin(); it != mStoreManager.actionsEnd(); ++it)
        mIds.push_back(it.getActionId());

    endResetModel();
}


int ActionsModel::rowCount(const QModelIndex &parent) const {
    return mIds.size();
}

int ActionsModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant ActionsModel::data(const QModelIndex &index, int role) const {
    auto actionId = getAction(index.row());
    const Action &action = mStoreManager.getAction(actionId);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return QString::fromStdString(actionId.getClientId().toString());
            case 1:
                return QString::fromStdString(action.getDescription());
        }
    }

    if (role == Qt::UserRole) { // Used for sorting
        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

QVariant ActionsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString(tr("Client"));
                case 1:
                    return QString(tr("Description"));
            }
        }
    }
    return QVariant();
}

std::vector<ClientActionId> ActionsModel::getActions(const QItemSelection &selection) const {
    std::unordered_set<int> rows;
    for (auto index : selection.indexes())
        rows.insert(index.row());

    std::vector<ClientActionId> actionIds;
    for (auto row : rows)
        actionIds.push_back(getAction(row));

    return std::move(actionIds);
}

ClientActionId ActionsModel::getAction(int row) const {
    return mIds[row];
}

int ActionsModel::getRow(ClientActionId id) const {
    size_t row = 0;
    for (ClientActionId el : mIds) {
        if (el == id)
            break;
        ++row;
    }

    return row;
}

void ActionsModel::addAction(ClientActionId actionId, size_t pos) {
    beginInsertRows(QModelIndex(), pos, pos);
    mIds.insert(mIds.begin() + pos, actionId);
    endInsertRows();
}

void ActionsModel::eraseAction(ClientActionId actionId) {
    int row = getRow(actionId);
    beginRemoveRows(QModelIndex(), row, row);
    mIds.erase(mIds.begin() + row);
    endRemoveRows();
}

ActionsProxyModel::ActionsProxyModel(StoreManager &storeManager, QObject *parent)
    : QSortFilterProxyModel(parent)
    , mStoreManager(storeManager)
    , mHidden(false)
{
    mModel = new ActionsModel(storeManager, this);

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &ActionsProxyModel::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &ActionsProxyModel::endResetTournament);

    setSourceModel(mModel);
    setSortRole(Qt::UserRole);
}

std::vector<ClientActionId> ActionsProxyModel::getActions(const QItemSelection &selection) const {
    return mModel->getActions(mapSelectionToSource(selection));
}

void ActionsProxyModel::hideAll() {
    mHidden = true;
    invalidateFilter();
}

void ActionsProxyModel::setClient(std::optional<ClientId> clientId) {
    mHidden = false;
    mClientId = clientId;
    invalidateFilter();
}

void ActionsProxyModel::setMatch(std::optional<std::pair<CategoryId, MatchId>> matchId) {
    mHidden = false;
    mMatchId = matchId;;
    invalidateFilter();
}

bool ActionsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    if (mHidden) {
        return false;
    }

    auto actionId = mModel->getAction(sourceRow);

    if (mClientId) {
        if (mClientId != actionId.getClientId())
            return false;
    }

    if (mMatchId) {
        const Action &action = mStoreManager.getAction(actionId);
        if (!action.shouldDisplay(mMatchId->first, mMatchId->second))
            return false;

        const auto &tournament = mStoreManager.getTournament();
        if (!tournament.containsCategory(mMatchId->first))
            return false;
        const auto &category = tournament.getCategory(mMatchId->first);
        if (!category.containsMatch(mMatchId->second))
            return false;
        const auto &match = category.getMatch(mMatchId->second);
        if (!match.getWhitePlayer() || !match.getBluePlayer())
            return false;
    }

    return true;
}

void ActionsProxyModel::beginResetTournament() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void ActionsProxyModel::endResetTournament() {
    auto &tournament = mStoreManager.getTournament();
    mConnections.push(connect(&tournament, &QTournamentStore::matchesAboutToBeReset, this, &ActionsProxyModel::beginResetMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesReset, this, &ActionsProxyModel::endResetMatches));
    invalidateFilter();
}

void ActionsProxyModel::beginResetMatches(CategoryId categoryId) {
    if (mMatchId && mMatchId->first == categoryId)
        invalidateFilter();
}

void ActionsProxyModel::endResetMatches(CategoryId categoryId) {
    if (mMatchId && mMatchId->first == categoryId)
        invalidateFilter();
}

