#include "actions/tournament_actions.hpp"
#include "stores/tournament_store.hpp"
#include "exception.hpp"

ChangeTournamentNameAction::ChangeTournamentNameAction(TournamentStore & tournament, const std::string & name)
    : mName(name)
{}

void ChangeTournamentNameAction::redoImpl(TournamentStore & tournament) {
    swapNames(tournament);
}

void ChangeTournamentNameAction::undoImpl(TournamentStore & tournament) {
    swapNames(tournament);
}

void ChangeTournamentNameAction::swapNames(TournamentStore & tournament) {
    std::string oldName = tournament.getName();
    tournament.setName(mName);

    mName = oldName;
    tournament.tournamentChanged();
}