#include "widgets/models/players_model.hpp"

PlayersModel::PlayersModel(std::unique_ptr<QTournamentStore> & tournament, QObject * parent)
    : QAbstractTableModel(parent)
    , mTournament(tournament)
{
    QObject::connect(mTournament.get(), &QTournamentStore::playerAddedSignal, this, &PlayersModel::playerAdded);
}

void PlayersModel::tournamentChanged(Id id) {
    // TODO
}

void PlayersModel::matchAdded(Id id) {
    // TODO
}

void PlayersModel::matchChanged(Id id) {
    // TODO
}

void PlayersModel::matchDeleted(Id id) {
    // TODO
}

void PlayersModel::categoryAdded(Id id) {
    // TODO
}

void PlayersModel::categoryChanged(Id id) {
    // TODO
}

void PlayersModel::categoryDeleted(Id id) {
    // TODO
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

