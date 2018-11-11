#include "widgets/models/categories_model.hpp"
#include "stores/qtournament_store.hpp"

CategoriesModel::CategoriesModel(StoreManager & storeManager, QObject * parent)
    : QAbstractTableModel(parent)
    , mStoreManager(storeManager)
{
    tournamentReset();
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

void CategoriesModel::tournamentReset() {
    beginResetModel();
    QTournamentStore & tournament = mStoreManager.getTournament();

    mIds.clear();
    for (const auto & p : tournament.getCategories())
        mIds.insert(p.first);

    connect(&tournament, &QTournamentStore::categoriesAdded, this, &CategoriesModel::categoriesAdded);
    connect(&tournament, &QTournamentStore::categoriesChanged, this, &CategoriesModel::categoriesChanged);
    connect(&tournament, &QTournamentStore::categoriesAboutToBeErased, this, &CategoriesModel::categoriesAboutToBeErased);
    connect(&tournament, &QTournamentStore::categoriesAboutToBeReset, this, &CategoriesModel::categoriesAboutToBeReset);
    connect(&tournament, &QTournamentStore::categoriesReset, this, &CategoriesModel::categoriesReset);
    connect(&tournament, &QTournamentStore::matchesReset, this, &CategoriesModel::matchesReset);
    connect(&tournament, &QTournamentStore::playersAddedToCategory, [&](CategoryId categoryId, const std::vector<PlayerId> &playerIds) {this->categoryPlayersChanged(categoryId);});
    connect(&tournament, &QTournamentStore::playersErasedFromCategory, [&](CategoryId categoryId, const std::vector<PlayerId> &playerIds) {this->categoryPlayersChanged(categoryId);});
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &CategoriesModel::tournamentReset);

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
{
    mModel = new CategoriesModel(storeManager, this);

    setSourceModel(mModel);
    setSortRole(Qt::UserRole);
}

std::vector<CategoryId> CategoriesProxyModel::getCategories(const QItemSelection &selection) const {
    return mModel->getCategories(mapSelectionToSource(selection));
}

