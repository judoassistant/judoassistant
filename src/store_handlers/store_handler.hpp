#pragma once

#include "core.hpp"
#include "stores/tournament_store.hpp"

class Action;

class StoreHandler {
public:
    virtual ~StoreHandler() {}
    virtual TournamentStore & getTournament() = 0;
    virtual const TournamentStore & getTournament() const = 0;
    virtual void dispatch(std::unique_ptr<Action> && action) = 0;
    virtual bool canUndo() = 0;
    virtual void undo() = 0;
    virtual bool canRedo() = 0;
    virtual void redo() = 0;
};
