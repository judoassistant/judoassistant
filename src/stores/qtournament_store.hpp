#pragma once

#include <QObject>

#include "core.hpp"
#include "serialize.hpp"
#include "stores/tournament_store.hpp"

class QTournamentStore : public QObject {
    Q_OBJECT
public:
    QTournamentStore() {}
signals:
    // signals useful for Qt
    // void tournamentChanged(Id id);
    // void matchAdded(Id id);
    // void matchChanged(Id id);
    // void matchDeleted(Id id);
    // void categoryAdded(Id id);
    // void categoryChanged(Id id);
    // void categoryDeleted(Id id);
    void playerAddedSignal(Id);
    // void playerChanged(Id id);
    // void playerDeleted(Id id);
};

