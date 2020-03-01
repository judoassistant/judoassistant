#include "core/actions/change_tournament_location_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangeTournamentLocationAction::ChangeTournamentLocationAction(const std::string & location)
    : mLocation(location)
{}

void ChangeTournamentLocationAction::redoImpl(TournamentStore & tournament) {
    mOldLocation = tournament.getLocation();
    tournament.setLocation(mLocation);
    tournament.changeTournament();
}

void ChangeTournamentLocationAction::undoImpl(TournamentStore & tournament) {
    tournament.setLocation(mOldLocation);
    tournament.changeTournament();
}

std::unique_ptr<Action> ChangeTournamentLocationAction::freshClone() const {
    return std::make_unique<ChangeTournamentLocationAction>(mLocation);
}

std::string ChangeTournamentLocationAction::getDescription() const {
    return "Change tournament location";
}
