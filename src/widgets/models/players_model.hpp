#pragma once
#include <QAbstractTableModel>
#include "stores/qtournament_store.hpp"

class PlayersModel : public QAbstractTableModel {
    Q_OBJECT
public:
    PlayersModel(std::unique_ptr<QTournamentStore> & tournament, QObject * parent);

public slots:
    void tournamentChanged(Id id);
    void matchAdded(Id id);
    void matchChanged(Id id);
    void matchDeleted(Id id);
    void categoryAdded(Id id);
    void categoryChanged(Id id);
    void categoryDeleted(Id id);
    void playerAdded(Id id);
    void playerChanged(Id id);
    void playerDeleted(Id id);
private:
    std::unique_ptr<QTournamentStore> & mTournament;
};
