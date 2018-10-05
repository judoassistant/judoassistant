#include "actions/composite_action.hpp"

CompositeAction::CompositeAction(std::vector<std::unique_ptr<Action>> && actions)
    : mActions(std::move(actions))
{}

void CompositeAction::redoImpl(TournamentStore & tournament) {
    // TODO: handle exceptions atomicly
    for (auto it = mActions.begin(); it != mActions.end(); ++it)
        (*it)->redo(tournament);
}

void CompositeAction::undoImpl(TournamentStore & tournament) {
    // TODO: undo in reverse order
    for (auto it = mActions.rbegin(); it != mActions.rend(); ++it)
        (*it)->undo(tournament);
}
