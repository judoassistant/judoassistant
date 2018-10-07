#include "widgets/models/players_model.hpp"
#include "stores/qtournament_store.hpp"

PlayersModel::PlayersModel(QStoreHandler & storeHandler, QObject * parent)
    : QAbstractTableModel(parent)
    , mStoreHandler(storeHandler)
{
    tournamentReset();
}

int PlayersModel::rowCount(const QModelIndex &parent) const {
    return mStoreHandler.getTournament().getPlayers().size();
}

int PlayersModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant PlayersModel::data(const QModelIndex &index, int role) const {
    auto playerId = getPlayer(index.row());
    const PlayerStore &player = mStoreHandler.getTournament().getPlayer(playerId);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return QString(QString::fromStdString(player.getFirstName()));
            case 1:
                return QString(QString::fromStdString(player.getLastName()));
            case 2:
                return (player.getAge().has_value() ? QVariant(player.getAge().value()) : QVariant(""));
            case 3:
                return (player.getRank().has_value() ? QVariant(QString::fromStdString(player.getRank().value().toString())) : QVariant(""));
            case 4:
                return QString(QString::fromStdString(player.getClub()));
            case 5:
                return (player.getWeight().has_value() ? QVariant(player.getWeight().value()) : QVariant(""));
            case 6:
                return (player.getCountry().has_value() ? QVariant(QString::fromStdString(player.getCountry().value().toString())) : QVariant(""));
        }
    }

    if (role == Qt::UserRole) { // Used for sorting
        if (index.column() == 3)
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
                    return QString(tr("First name"));
                case 1:
                    return QString(tr("Last name"));
                case 2:
                    return QString(tr("Age"));
                case 3:
                    return QString(tr("Rank"));
                case 4:
                    return QString(tr("Club"));
                case 5:
                    return QString(tr("Weight"));
                case 6:
                    return QString(tr("Country"));
            }
        }
    }
    return QVariant();
}

std::vector<Id> PlayersModel::getPlayers(const QItemSelection &selection) const {
    std::unordered_set<int> rows;
    for (auto index : selection.indexes())
        rows.insert(index.row());

    std::vector<Id> playerIds;
    for (auto row : rows)
        playerIds.push_back(getPlayer(row));

    return std::move(playerIds);
}

Id PlayersModel::getPlayer(int row) const {
    auto it = mIds.begin();
    std::advance(it, row);
    return *it;
}

int PlayersModel::getRow(Id id) const {
    return std::distance(mIds.begin(), mIds.lower_bound(id));
}

void PlayersModel::playersAdded(std::vector<Id> ids) {
    for (auto id : ids) {
        int row = getRow(id);
        beginInsertRows(QModelIndex(), row, row);
        mIds.insert(id);
        endInsertRows();
    }
}

void PlayersModel::playersChanged(std::vector<Id> ids) {
    for (auto id : ids) {
        int row = getRow(id);
        emit dataChanged(createIndex(0,row), createIndex(COLUMN_COUNT-1,row));
    }
}

void PlayersModel::playersAboutToBeErased(std::vector<Id> ids) {
    for (auto id : ids) {
        int row = getRow(id);
        beginRemoveRows(QModelIndex(), row, row);
        mIds.erase(id);
        endRemoveRows();
    }
}

void PlayersModel::playersAboutToBeReset() {
    beginResetModel();
}

void PlayersModel::playersReset() {
    mIds.clear();
    for (const auto & p : mStoreHandler.getTournament().getPlayers())
        mIds.insert(p.first);
    endResetModel();
}

void PlayersModel::tournamentReset() {
    beginResetModel();
    QTournamentStore & tournament = mStoreHandler.getTournament();

    mIds.clear();
    for (const auto & p : tournament.getPlayers())
        mIds.insert(p.first);

    connect(&tournament, &QTournamentStore::playersAdded, this, &PlayersModel::playersAdded);
    connect(&tournament, &QTournamentStore::playersChanged, this, &PlayersModel::playersChanged);
    connect(&tournament, &QTournamentStore::playersAboutToBeErased, this, &PlayersModel::playersAboutToBeErased);
    connect(&tournament, &QTournamentStore::playersAboutToBeReset, this, &PlayersModel::playersAboutToBeReset);
    connect(&tournament, &QTournamentStore::playersReset, this, &PlayersModel::playersReset);
    connect(&mStoreHandler, &QStoreHandler::tournamentReset, this, &PlayersModel::tournamentReset);

    endResetModel();
}

