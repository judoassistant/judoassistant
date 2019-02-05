#pragma once

#include <list>
#include <unordered_set>

#include <QObject>

#include "core/actions/action.hpp"
#include "core/core.hpp"
#include "core/id.hpp"

class QTournamentStore;

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

