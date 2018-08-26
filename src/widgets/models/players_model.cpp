#include "widgets/models/players_model.hpp"
#include "stores/qtournament_store.hpp"

PlayersModel::PlayersModel(QStoreHandler & storeHandler, QObject * parent)
    : QAbstractTableModel(parent)
    , mStoreHandler(storeHandler)
{
    QTournamentStore & tournament = storeHandler.getTournament();

    QObject::connect(&tournament, &QTournamentStore::playerAdded, this, &PlayersModel::playerAdded);
    QObject::connect(&tournament, &QTournamentStore::playerChanged, this, &PlayersModel::playerChanged);
    QObject::connect(&tournament, &QTournamentStore::playerDeleted, this, &PlayersModel::playerDeleted);
    QObject::connect(&mStoreHandler, &QStoreHandler::tournamentReset, this, &PlayersModel::tournamentReset);
}

void PlayersModel::playerAdded(Id id) {
    // TODO
}

void PlayersModel::playerChanged(Id id) {
    // TODO
}

void PlayersModel::playerDeleted(Id id) {
    // TODO
}

void PlayersModel::tournamentReset() {
    // TODO
}

int PlayersModel::rowCount(const QModelIndex &parent) const {
    return mStoreHandler.getTournament().getPlayers().size();
}

int PlayersModel::columnCount(const QModelIndex &parent) const {
    return 3;
}

QVariant PlayersModel::data(const QModelIndex &index, int role) const {
    // TODO
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
            }
        }
    }
    return QVariant();
}

