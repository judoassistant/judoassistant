#pragma once

#include <list>
#include <unordered_set>

#include <QObject>

#include "core.hpp"
#include "id.hpp"

class Action;
class QTournamentStore;

typedef std::list<std::pair<ClientActionId, std::shared_ptr<Action>>> SharedActionList;
typedef std::list<std::pair<ClientActionId, std::unique_ptr<Action>>> UniqueActionList;

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

Q_DECLARE_METATYPE(SyncPayloadPtr)
Q_DECLARE_METATYPE(ClientActionId)
Q_DECLARE_METATYPE(ActionPtr)
Q_DECLARE_METATYPE(OptionalClientActionId)

void registerMetatypes();

