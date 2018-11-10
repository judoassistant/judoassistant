#include "action.hpp"

Action::Action() : mDone(false)
{}

void Action::redo(TournamentStore &tournament) {
    if (mDone)
        throw ActionExecutionException("The action has already been executed.");
    mDone = true;
    redoImpl(tournament);
}

void Action::undo(TournamentStore &tournament) {
    if (!mDone)
        throw ActionExecutionException("The action has not been executed.");
    mDone = false;
    undoImpl(tournament);
}

bool Action::isDone() const {
    return mDone;
}

