#include "actions/tournament_actions.hpp"
#include "stores/tournament_store.hpp"
#include "exception.hpp"

ChangeTournamentNameAction::ChangeTournamentNameAction(TournamentStore & tournament, const std::string & name)
    : mName(name)
{}

void ChangeTournamentNameAction::redoImpl(TournamentStore & tournament) {
    mOldName = tournament.getName();
    tournament.setName(mName);
    tournament.changeTournament();
}

void ChangeTournamentNameAction::undoImpl(TournamentStore & tournament) {
    tournament.setName(mOldName);
    tournament.changeTournament();
}

