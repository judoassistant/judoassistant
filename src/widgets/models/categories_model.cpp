#include "widgets/models/categories_model.hpp"
#include "stores/qtournament_store.hpp"

CategoriesModel::CategoriesModel(QStoreHandler & storeHandler, QObject * parent)
    : QAbstractTableModel(parent)
    , mStoreHandler(storeHandler)
{
    tournamentReset();
}

void CategoriesModel::categoryAdded(Id id) {
    auto it = mIds.insert(id).first;
    int row = std::distance(mIds.begin(), it);
    beginInsertRows(QModelIndex(), row, row);
    endInsertRows();
}

void CategoriesModel::categoryChanged(Id id) {
    auto it = mIds.find(id);
    int row = std::distance(mIds.begin(), it);

    emit dataChanged(createIndex(0,row), createIndex(COLUMN_COUNT-1,row));
}

void CategoriesModel::categoryDeleted(Id id) {
    auto it = mIds.find(id);
    int row = std::distance(mIds.begin(), it);
    beginRemoveRows(QModelIndex(), row, row);
    mIds.erase(it);
    endRemoveRows();
}

void CategoriesModel::tournamentReset() {
    beginResetModel();
    QTournamentStore & tournament = mStoreHandler.getTournament();

    mIds.clear();
    for (const auto & p : tournament.getCategories())
        mIds.insert(p.first);

    QObject::connect(&tournament, &QTournamentStore::categoryAdded, this, &CategoriesModel::categoryAdded);
    QObject::connect(&tournament, &QTournamentStore::categoryChanged, this, &CategoriesModel::categoryChanged);
    QObject::connect(&tournament, &QTournamentStore::categoryDeleted, this, &CategoriesModel::categoryDeleted);
    QObject::connect(&mStoreHandler, &QStoreHandler::tournamentReset, this, &CategoriesModel::tournamentReset);
    endResetModel();
}

int CategoriesModel::rowCount(const QModelIndex &parent) const {
    return mStoreHandler.getTournament().getCategories().size();
}

int CategoriesModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant CategoriesModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        auto it = mIds.begin();
        std::advance(it, index.row());

        Id id = *it;
        CategoryStore &category = mStoreHandler.getTournament().getCategory(id);
        switch (index.column()) {
            case 0:
                return QString(QString::fromStdString(category.getName()));
            case 1:
                return QString(QString::fromStdString(category.getRuleset().getName()));
            case 2:
                return QString(QString::fromStdString(category.getDrawStrategy().getName()));
            case 3:
                return static_cast<int>(category.getPlayers().size());
        }
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
            }
        }
    }
    return QVariant();
}

