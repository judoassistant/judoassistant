#pragma once

#include <QObject>

#include "core/core.hpp"
#include "core/id.hpp"
#include "ui/store_managers/sync_payload.hpp"

class QTournamentStore;
class TournamentStore;
class Action;

class NetworkInterface : public QObject {
    Q_OBJECT
public:
    virtual void postSync(std::unique_ptr<TournamentStore> tournament) = 0;
    virtual void postAction(ClientActionId actionId, std::unique_ptr<Action> action) = 0;
    virtual void postUndo(ClientActionId actionId) = 0;

    virtual void stop() = 0;

signals:
    void actionReceived(ClientActionId actionId, ActionPtr action);
    void actionConfirmReceived(ClientActionId actionId);
    void undoReceived(ClientActionId actionId);
    void undoConfirmReceived(ClientActionId actionId);
    void syncConfirmed();
    void syncReceived(SyncPayloadPtr syncPayload);
};

