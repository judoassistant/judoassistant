#include "widgets/models/players_model.hpp"
#include "stores/qtournament_store.hpp"

PlayersModel::PlayersModel(QStoreHandler & storeHandler, QObject * parent)
    : QAbstractTableModel(parent)
    , mStoreHandler(storeHandler)
{
    tournamentReset();
}

void PlayersModel::playerAdded(Id id) {
    auto it = mIds.insert(id).first;
    int row = std::distance(mIds.begin(), it);
    beginInsertRows(QModelIndex(), row, row);
    endInsertRows();
}

void PlayersModel::playerChanged(Id id) {
    auto it = mIds.find(id);
    int row = std::distance(mIds.begin(), it);

    emit dataChanged(createIndex(0,row), createIndex(COLUMN_COUNT-1,row));
}

void PlayersModel::playerDeleted(Id id) {
    auto it = mIds.find(id);
    int row = std::distance(mIds.begin(), it);
    beginRemoveRows(QModelIndex(), row, row);
    mIds.erase(it);
    endRemoveRows();
}

void PlayersModel::tournamentReset() {
    beginResetModel();
    QTournamentStore & tournament = mStoreHandler.getTournament();

    mIds.clear();
    for (const auto & p : tournament.getPlayers())
        mIds.insert(p.first);

    QObject::connect(&tournament, &QTournamentStore::playerAdded, this, &PlayersModel::playerAdded);
    QObject::connect(&tournament, &QTournamentStore::playerChanged, this, &PlayersModel::playerChanged);
    QObject::connect(&tournament, &QTournamentStore::playerDeleted, this, &PlayersModel::playerDeleted);
    QObject::connect(&mStoreHandler, &QStoreHandler::tournamentReset, this, &PlayersModel::tournamentReset);
    endResetModel();
}

int PlayersModel::rowCount(const QModelIndex &parent) const {
    return mStoreHandler.getTournament().getPlayers().size();
}

int PlayersModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant PlayersModel::data(const QModelIndex &index, int role) const {
    // TODO
    if (role == Qt::DisplayRole) {
        auto it = mIds.begin();
        std::advance(it, index.row());

        Id id = *it;
        PlayerStore &player = mStoreHandler.getTournament().getPlayer(id);
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

