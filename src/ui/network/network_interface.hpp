#pragma once

#include <QObject>

#include "core/actions/action.hpp"
#include "core/core.hpp"
#include "core/id.hpp"

class QTournamentStore;
class TournamentStore;
class Action;

struct SyncPayload {
    SyncPayload(std::unique_ptr<QTournamentStore> tournament, std::unique_ptr<UniqueActionList> confirmedActionList, std::unique_ptr<UniqueActionList> unconfirmedActionList, std::unique_ptr<std::unordered_set<ClientActionId>> unconfirmedUndos);
    std::unique_ptr<QTournamentStore> tournament;
    std::unique_ptr<UniqueActionList> confirmedActionList;
    std::unique_ptr<UniqueActionList> unconfirmedActionList;
    std::unique_ptr<std::unordered_set<ClientActionId>> unconfirmedUndos;
};

typedef std::shared_ptr<SyncPayload> SyncPayloadPtr;
typedef std::shared_ptr<Action> ActionPtr;
typedef std::optional<ClientActionId> OptionalClientActionId;

class NetworkInterface : public QObject {
    Q_OBJECT
public:
    NetworkInterface();

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

Q_DECLARE_METATYPE(SyncPayloadPtr)
Q_DECLARE_METATYPE(ClientActionId)
Q_DECLARE_METATYPE(ActionPtr)
Q_DECLARE_METATYPE(OptionalClientActionId)

