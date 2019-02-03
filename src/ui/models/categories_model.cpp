#include "core/stores/category_store.hpp"
#include "ui/misc/numerical_string_comparator.hpp"
#include "ui/models/categories_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"

CategoriesModel::CategoriesModel(StoreManager & storeManager, QObject * parent)
    : QAbstractTableModel(parent)
    , mStoreManager(storeManager)
{
    tournamentAboutToBeReset();
    tournamentReset();

    connect(&mStoreManager, &StoreManager::tournamentReset, this, &CategoriesModel::tournamentReset);
    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &CategoriesModel::tournamentAboutToBeReset);
}

int CategoriesModel::rowCount(const QModelIndex &parent) const {
    return mStoreManager.getTournament().getCategories().size();
}

int CategoriesModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant CategoriesModel::data(const QModelIndex &index, int role) const {
    auto categoryId = getCategory(index.row());
    const CategoryStore &category = mStoreManager.getTournament().getCategory(categoryId);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return QString(QString::fromStdString(category.getName()));
            case 1:
                return QString(QString::fromStdString(category.getRuleset().getName()));
            case 2:
                return QString(QString::fromStdString(category.getDrawSystem().getName()));
            case 3:
                return static_cast<int>(category.getPlayers().size());
            case 4:
                return static_cast<int>(category.getMatches().size());
        }
    }

    if (role == Qt::UserRole) { // Used for sorting
        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

QVariant CategoriesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString(tr("Name"));
                case 1:
                    return QString(tr("Ruleset"));
                case 2:
                    return QString(tr("Draw System"));
                case 3:
                    return QString(tr("Players Count"));
                case 4:
                    return QString(tr("Match Count"));
            }
        }
    }
    return QVariant();
}

std::vector<CategoryId> CategoriesModel::getCategories(const QItemSelection &selection) const {
    std::unordered_set<int> rows;
    for (auto index : selection.indexes())
        rows.insert(index.row());

    std::vector<CategoryId> categoryIds;
    for (auto row : rows)
        categoryIds.push_back(getCategory(row));

    return std::move(categoryIds);
}

CategoryId CategoriesModel::getCategory(int row) const {
    auto it = mIds.begin();
    std::advance(it, row);
    return *it;
}

int CategoriesModel::getRow(CategoryId id) const {
    return std::distance(mIds.begin(), mIds.lower_bound(id));
}

void CategoriesModel::categoriesAdded(std::vector<CategoryId> ids) {
    for (auto id : ids) {
        int row = getRow(id);
        beginInsertRows(QModelIndex(), row, row);
        mIds.insert(id);
        endInsertRows();
    }
}

void CategoriesModel::categoriesChanged(std::vector<CategoryId> ids) {
    for (auto id : ids) {
        int row = getRow(id);
        emit dataChanged(createIndex(0,row), createIndex(COLUMN_COUNT-1,row));
    }
}

void CategoriesModel::categoriesAboutToBeErased(std::vector<CategoryId> ids) {
    for (auto id : ids) {
        int row = getRow(id);
        beginRemoveRows(QModelIndex(), row, row);
        mIds.erase(id);
        endRemoveRows();
    }
}

void CategoriesModel::categoriesAboutToBeReset() {
    beginResetModel();
}

void CategoriesModel::categoriesReset() {
    mIds.clear();
    for (const auto & p : mStoreManager.getTournament().getCategories())
        mIds.insert(p.first);
    endResetModel();
}

void CategoriesModel::tournamentAboutToBeReset() {
    beginResetModel();

    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void CategoriesModel::tournamentReset() {
    beginResetModel();
    QTournamentStore & tournament = mStoreManager.getTournament();

    mIds.clear();
    for (const auto & p : tournament.getCategories())
        mIds.insert(p.first);

    mConnections.push(connect(&tournament, &QTournamentStore::categoriesAdded, this, &CategoriesModel::categoriesAdded));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesChanged, this, &CategoriesModel::categoriesChanged));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesAboutToBeErased, this, &CategoriesModel::categoriesAboutToBeErased));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesAboutToBeReset, this, &CategoriesModel::categoriesAboutToBeReset));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesReset, this, &CategoriesModel::categoriesReset));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesReset, this, &CategoriesModel::matchesReset));
    mConnections.push(connect(&tournament, &QTournamentStore::playersAddedToCategory, [&](CategoryId categoryId, const std::vector<PlayerId> &playerIds) {this->categoryPlayersChanged(categoryId);}));
    mConnections.push(connect(&tournament, &QTournamentStore::playersErasedFromCategory, [&](CategoryId categoryId, const std::vector<PlayerId> &playerIds) {this->categoryPlayersChanged(categoryId);}));

    endResetModel();
}

void CategoriesModel::matchesReset(CategoryId categoryId) {
    int row = getRow(categoryId);
    emit dataChanged(createIndex(row, 4), createIndex(row, 4));
}

void CategoriesModel::categoryPlayersChanged(CategoryId categoryId) {
    int row = getRow(categoryId);
    emit dataChanged(createIndex(row, 3), createIndex(row, 3));
}

CategoriesProxyModel::CategoriesProxyModel(StoreManager &storeManager, QObject *parent)
    : QSortFilterProxyModel(parent)
    , mStoreManager(storeManager)
{
    mModel = new CategoriesModel(mStoreManager, this);

    setSourceModel(mModel);
    setSortRole(Qt::UserRole);
}

std::vector<CategoryId> CategoriesProxyModel::getCategories(const QItemSelection &selection) const {
    return mModel->getCategories(mapSelectionToSource(selection));
}

bool CategoriesProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    if (left.column() == 0 && right.column() == 0) {
        const auto &tournament = mStoreManager.getTournament();
        const auto &leftCategory = tournament.getCategory(mModel->getCategory(left.row()));
        const auto &rightCategory = tournament.getCategory(mModel->getCategory(right.row()));

        NumericalStringComparator comp;
        return comp(leftCategory.getName(), rightCategory.getName());
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

