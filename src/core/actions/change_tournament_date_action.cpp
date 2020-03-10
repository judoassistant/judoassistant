#include "core/actions/change_tournament_date_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangeTournamentDateAction::ChangeTournamentDateAction(const std::string & date)
    : mDate(date)
{}

void ChangeTournamentDateAction::redoImpl(TournamentStore & tournament) {
    mOldDate = tournament.getDate();
    tournament.setDate(mDate);
    tournament.changeTournament();
}

void ChangeTournamentDateAction::undoImpl(TournamentStore & tournament) {
    tournament.setDate(mOldDate);
    tournament.changeTournament();
}

std::unique_ptr<Action> ChangeTournamentDateAction::freshClone() const {
    return std::make_unique<ChangeTournamentDateAction>(mDate);
}

std::string ChangeTournamentDateAction::getDescription() const {
    return "Change tournament date";
}
