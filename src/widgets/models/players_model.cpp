#include "widgets/models/players_model.hpp"

PlayersModel::PlayersModel(std::unique_ptr<QTournamentStore> & tournament, QObject * parent)
    : QAbstractTableModel(parent)
    , mTournament(tournament)
{
    QObject::connect(mTournament.get(), &QTournamentStore::playerAdded, this, &PlayersModel::playerAdded);
    QObject::connect(mTournament.get(), &QTournamentStore::playerChanged, this, &PlayersModel::playerChanged);
    QObject::connect(mTournament.get(), &QTournamentStore::playerDeleted, this, &PlayersModel::playerDeleted);
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

int PlayersModel::rowCount(const QModelIndex &parent) const {
    return mTournament->getPlayers().size();
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

