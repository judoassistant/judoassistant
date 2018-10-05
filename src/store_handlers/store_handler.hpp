#pragma once

#include "core.hpp"
#include "stores/tournament_store.hpp"

class Action;

class StoreHandler {
public:
    virtual ~StoreHandler() {}
    virtual void dispatch(std::unique_ptr<Action> && action) = 0;
    virtual TournamentStore & getTournament() = 0;
};
