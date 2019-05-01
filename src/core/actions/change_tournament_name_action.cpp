#include "core/actions/change_tournament_name_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangeTournamentNameAction::ChangeTournamentNameAction(const std::string & name)
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

std::unique_ptr<Action> ChangeTournamentNameAction::freshClone() const {
    return std::make_unique<ChangeTournamentNameAction>(mName);
}

std::string ChangeTournamentNameAction::getDescription() const {
    return "Change tournament name";
}
