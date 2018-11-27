#pragma once

#include <list>
#include <unordered_set>

#include <QObject>

#include "core.hpp"
#include "id.hpp"

class Action;
class QTournamentStore;

// TODO: Include ClientId in action list types
typedef std::list<std::pair<ActionId, std::shared_ptr<Action>>> SharedActionList;
typedef std::list<std::pair<ActionId, std::unique_ptr<Action>>> UniqueActionList;

struct SyncPayload {
    SyncPayload(std::unique_ptr<QTournamentStore> tournament, std::unique_ptr<UniqueActionList> confirmedActionList, std::unique_ptr<UniqueActionList> unconfirmedActionList, std::unique_ptr<std::unordered_set<ActionId>> unconfirmedUndos);
    std::unique_ptr<QTournamentStore> tournament;
    std::unique_ptr<UniqueActionList> confirmedActionList;
    std::unique_ptr<UniqueActionList> unconfirmedActionList;
    std::unique_ptr<std::unordered_set<ActionId>> unconfirmedUndos;
};

typedef std::shared_ptr<SyncPayload> SyncPayloadPtr;
typedef std::shared_ptr<Action> ActionPtr;

Q_DECLARE_METATYPE(SyncPayloadPtr);
Q_DECLARE_METATYPE(ActionId)
Q_DECLARE_METATYPE(ActionPtr)

void registerMetatypes();

