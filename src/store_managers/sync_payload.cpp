#include "actions/action.hpp"
#include "store_managers/sync_payload.hpp"
#include "stores/qtournament_store.hpp"

SyncPayload::SyncPayload(std::unique_ptr<QTournamentStore> tournament, std::unique_ptr<UniqueActionList> confirmedActionList, std::unique_ptr<UniqueActionList> unconfirmedActionList, std::unique_ptr<std::unordered_set<ClientActionId>> unconfirmedUndos)
    : tournament(std::move(tournament))
    , confirmedActionList(std::move(confirmedActionList))
    , unconfirmedActionList(std::move(unconfirmedActionList))
    , unconfirmedUndos(std::move(unconfirmedUndos))
{}

void registerMetatypes() {
    qRegisterMetaType<SyncPayloadPtr>();
    qRegisterMetaType<ActionPtr>();
    qRegisterMetaType<ClientActionId>();
    qRegisterMetaType<OptionalClientActionId>();
}
