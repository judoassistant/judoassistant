#include <QColor>
#include <QBrush>
#include <sstream>

#include "core/draw_systems/draw_system.hpp"
#include "core/id.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/player_store.hpp"
#include "ui/misc/numerical_string_comparator.hpp"
#include "ui/models/players_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"

PlayersModel::PlayersModel(StoreManager & storeManager, QObject * parent)
    : QAbstractTableModel(parent)
    , mStoreManager(storeManager)
{
    tournamentAboutToBeReset();
    tournamentReset();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &PlayersModel::tournamentAboutToBeReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &PlayersModel::tournamentReset);
}

int PlayersModel::rowCount(const QModelIndex &parent) const {
    return mStoreManager.getTournament().getPlayers().size();
}

int PlayersModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant PlayersModel::data(const QModelIndex &index, int role) const {
    auto playerId = getPlayer(index.row());
    const PlayerStore &player = mStoreManager.getTournament().getPlayer(playerId);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return QString::fromStdString(player.getFirstName());
            case 1:
                return QString::fromStdString(player.getLastName());
            case 2:
                return (player.getSex() ? QVariant(QString::fromStdString(player.getSex()->toString())) : QVariant(""));
            case 3:
                return (player.getAge() ? QVariant(player.getAge()->toInt()) : QVariant(""));
            case 4:
                return (player.getWeight() ? QVariant(player.getWeight()->toFloat()) : QVariant(""));
            case 5:
                return (player.getRank() ? QVariant(QString::fromStdString(player.getRank()->toString())) : QVariant(""));
            case 6:
                return QString(QString::fromStdString(player.getClub()));
            case 7:
                return (player.getCountry() ? QVariant(QString::fromStdString(player.getCountry()->toString())) : QVariant(""));
            case 8:
                return QString::fromStdString(listPlayerCategories(player));
        }
    }

    if (role == Qt::UserRole) { // Used for sorting
        if (index.column() == 5)
            return (player.getRank().has_value() ? QVariant(player.getRank().value().toInt()) : QVariant(""));
        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

QVariant PlayersModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString(tr("First Name"));
                case 1:
                    return QString(tr("Last Name"));
                case 2:
                    return QString(tr("Sex"));
                case 3:
                    return QString(tr("Age"));
                case 4:
                    return QString(tr("Weight"));
                case 5:
                    return QString(tr("Rank"));
                case 6:
                    return QString(tr("Club"));
                case 7:
                    return QString(tr("Country"));
                case 8:
                    return QString(tr("Categories"));
            }
        }
    }
    return QVariant();
}

std::vector<PlayerId> PlayersModel::getPlayers(const QItemSelection &selection) const {
    std::unordered_set<int> rows;
    for (auto index : selection.indexes())
        rows.insert(index.row());

    std::vector<PlayerId> playerIds;
    for (auto row : rows)
        playerIds.push_back(getPlayer(row));

    return playerIds;
}

PlayerId PlayersModel::getPlayer(int row) const {
    auto it = mIds.begin();
    std::advance(it, row);
    return *it;
}

int PlayersModel::getRow(PlayerId playerId) const {
    return std::distance(mIds.begin(), mIds.lower_bound(playerId));
}

void PlayersModel::playersAdded(const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds) {
        int row = getRow(playerId);
        beginInsertRows(QModelIndex(), row, row);
        mIds.insert(playerId);
        endInsertRows();
    }
}

void PlayersModel::playersChanged(const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds) {
        int row = getRow(playerId);
        emit dataChanged(createIndex(row,0), createIndex(row, COLUMN_COUNT-1));
    }
}

void PlayersModel::playersAboutToBeErased(const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds) {
        int row = getRow(playerId);
        beginRemoveRows(QModelIndex(), row, row);
        mIds.erase(playerId);
        endRemoveRows();
    }
}

void PlayersModel::playersAboutToBeReset() {
    beginResetModel();
    mIds.clear();
}

void PlayersModel::playersReset() {
    for (const auto & p : mStoreManager.getTournament().getPlayers())
        mIds.insert(p.first);
    endResetModel();
}

void PlayersModel::tournamentAboutToBeReset() {
    beginResetModel();

    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void PlayersModel::tournamentReset() {
    QTournamentStore & tournament = mStoreManager.getTournament();

    mIds.clear();
    for (const auto & p : tournament.getPlayers())
        mIds.insert(p.first);

    mConnections.push(connect(&tournament, &QTournamentStore::playersAdded, this, &PlayersModel::playersAdded));
    mConnections.push(connect(&tournament, &QTournamentStore::playersChanged, this, &PlayersModel::playersChanged));
    mConnections.push(connect(&tournament, &QTournamentStore::playersAboutToBeErased, this, &PlayersModel::playersAboutToBeErased));
    mConnections.push(connect(&tournament, &QTournamentStore::playersAboutToBeReset, this, &PlayersModel::playersAboutToBeReset));
    mConnections.push(connect(&tournament, &QTournamentStore::playersReset, this, &PlayersModel::playersReset));

    mConnections.push(connect(&tournament, &QTournamentStore::playersAddedToCategory, this, qOverload<CategoryId, const std::vector<PlayerId>&>(&PlayersModel::playerCategoriesChanged)));
    mConnections.push(connect(&tournament, &QTournamentStore::playersErasedFromCategory, this, qOverload<CategoryId, const std::vector<PlayerId>&>(&PlayersModel::playerCategoriesChanged)));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesAboutToBeErased, this, &PlayersModel::categoriesAboutToBeErased));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesErased, this, &PlayersModel::categoriesErased));

    endResetModel();
}

std::string PlayersModel::listPlayerCategories(const PlayerStore &player) const {
    std::vector<std::string> names;
    for (auto categoryId : player.getCategories()) {
        const CategoryStore &category = mStoreManager.getTournament().getCategory(categoryId);
        names.push_back(category.getName());
    }

    std::sort(names.begin(), names.end(), NumericalStringComparator());

    std::stringstream res;
    for (size_t i = 0; i < names.size(); ++i) {
        res << names[i];
        if (i != names.size() - 1)
            res << ", ";
    }

    return res.str();
}

void PlayersModel::playerCategoriesChanged(const std::vector<PlayerId> &playerIds) {
    for (auto playerId : playerIds) {
        int row = getRow(playerId);
        emit dataChanged(createIndex(row, 7), createIndex(row, 7));
    }
}

void PlayersModel::playerCategoriesChanged(CategoryId categoryId, const std::vector<PlayerId> &playerIds) {
    playerCategoriesChanged(playerIds);
}

void PlayersModel::categoriesAboutToBeErased(const std::vector<CategoryId> &categoryIds) {
    mAffectedPlayers.clear();
    for (auto categoryId : categoryIds) {
        const auto & ids = mStoreManager.getTournament().getCategory(categoryId).getPlayers();
        mAffectedPlayers.insert(ids.begin(), ids.end());
    }
}

void PlayersModel::categoriesErased(const std::vector<CategoryId> &categoryIds) {
    playerCategoriesChanged(std::vector<PlayerId>(mAffectedPlayers.begin(), mAffectedPlayers.end()));
}

PlayersProxyModel::PlayersProxyModel(StoreManager &storeManager, QObject *parent)
    : QSortFilterProxyModel(parent)
    , mStoreManager(storeManager)
    , mHidden(false)
{
    mModel = new PlayersModel(storeManager, this);

    setSourceModel(mModel);
    setSortRole(Qt::UserRole);
}

std::vector<PlayerId> PlayersProxyModel::getPlayers(const QItemSelection &selection) const {
    return mModel->getPlayers(mapSelectionToSource(selection));
}

void PlayersProxyModel::hideAll() {
    mHidden = true;
    invalidateFilter();
}

void PlayersProxyModel::setCategory(std::optional<CategoryId> categoryId) {
    mHidden = false;
    mCategoryId = categoryId;
    invalidateFilter();
}

bool PlayersProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    if (mHidden)
        return false;
    if (!mCategoryId)
        return true;

    auto playerId = mModel->getPlayer(sourceRow);
    const auto &player = mStoreManager.getTournament().getPlayer(playerId);
    return player.containsCategory(*mCategoryId);
}

bool PlayersProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    if (left.column() == 8 && right.column() == 8) {
        std::string leftString = sourceModel()->data(left).toString().toStdString();
        std::string rightString = sourceModel()->data(right).toString().toStdString();

        NumericalStringComparator comp;
        return comp(leftString, rightString);
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

