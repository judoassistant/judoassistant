#pragma once

#include <QThread>

#include "core.hpp"
#include "id.hpp"
#include "store_managers/sync_payload.hpp"

class QTournamentStore;
class TournamentStore;
class Action;

class NetworkInterface : public QThread {
    Q_OBJECT
public:
    virtual void postSync(std::unique_ptr<TournamentStore> tournament) = 0;
    virtual void postAction(ActionId actionId, std::unique_ptr<Action> action) = 0;
    virtual void postUndo(ActionId actionId) = 0;

    virtual void start() = 0;
    virtual void quit() = 0;

signals:
    void actionReceived(ActionId actionId, ActionPtr action);
    void actionConfirmReceived(ActionId actionId);
    void undoReceived(ActionId actionId);
    void undoConfirmReceived(ActionId actionId);
    void syncConfirmed();
    void syncReceived(SyncPayloadPtr syncPayload);
};

