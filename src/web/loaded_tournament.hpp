#pragma once

#include "core/actions/action.hpp"
#include "core/stores/tournament_store.hpp"

class LoadedTournament {
public:
    void sync(std::unique_ptr<TournamentStore> tournament, SharedActionList actionList);

private:
    std::unique_ptr<TournamentStore> mTournament;
    SharedActionList mActionList;
};
