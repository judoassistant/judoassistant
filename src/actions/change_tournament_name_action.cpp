#include "actions/change_tournament_name_action.hpp"
#include "stores/tournament_store.hpp"
#include "exception.hpp"

ChangeTournamentNameAction::ChangeTournamentNameAction(TournamentStore & tournament, const std::string & name)
    : mNewName(name)
    , mOldName(tournament.getName())
{
}

ChangeTournamentNameAction::ChangeTournamentNameAction(const std::string & oldName, const std::string & newName)
    : mNewName(newName)
    , mOldName(oldName)
{
}

bool ChangeTournamentNameAction::operator()(TournamentStore & tournament) const {
    tournament.setName(mNewName);
    tournament.tournamentChanged();
    return true;
}

std::unique_ptr<Action> ChangeTournamentNameAction::getInverse() const {
    return std::make_unique<ChangeTournamentNameAction>(mNewName, mOldName);
}
