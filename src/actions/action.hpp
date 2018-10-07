#pragma once

#include "core.hpp"

// TODO: Implement serialization for all actions
class TournamentStore;

class Action {
public:
    Action();
    virtual ~Action() {}

    virtual void redoImpl(TournamentStore & tournament) = 0;
    virtual void undoImpl(TournamentStore & tournament) = 0;

    void redo(TournamentStore & tournament);
    void undo(TournamentStore & tournament);
private:
    bool mDone;
};

