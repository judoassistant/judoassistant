#include "actions/composite_action.hpp"

CompositeAction::CompositeAction(std::vector<std::unique_ptr<Action>> && actions)
    : mActions(std::move(actions))
{}

void CompositeAction::redoImpl(TournamentStore & tournament) {
    for (auto it = mActions.begin(); it != mActions.end(); ++it)
        (*it)->redo(tournament);
}

void CompositeAction::undoImpl(TournamentStore & tournament) {
    for (auto it = mActions.rbegin(); it != mActions.rend(); ++it)
        (*it)->undo(tournament);
}
