#pragma once

#include "core.hpp"
#include "store_handlers/store_handler.hpp"
#include "stores/qtournament_store.hpp"

class Action;

class QStoreHandler : public QObject, public StoreHandler {
    Q_OBJECT
public:
    QTournamentStore & getTournament() override = 0;
    const QTournamentStore & getTournament() const override = 0;
signals:
    void tournamentReset(); // TODO: change to abouttobereset
    void undoStatusChanged(bool canUndo);
    void redoStatusChanged(bool canRedo);
};
