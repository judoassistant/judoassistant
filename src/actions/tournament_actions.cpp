#include "actions/tournament_actions.hpp"
#include "stores/tournament_store.hpp"
#include "exception.hpp"

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

ChangeTournamentWebNameAction::ChangeTournamentWebNameAction(const std::string & name)
    : mWebName(name)
{}

void ChangeTournamentWebNameAction::redoImpl(TournamentStore & tournament) {
    mOldWebName = tournament.getWebName();
    tournament.setWebName(mWebName);
    tournament.changeTournament();
}

void ChangeTournamentWebNameAction::undoImpl(TournamentStore & tournament) {
    tournament.setWebName(mOldWebName);
    tournament.changeTournament();
}

std::unique_ptr<Action> ChangeTournamentWebNameAction::freshClone() const {
    return std::make_unique<ChangeTournamentWebNameAction>(mWebName);
}

std::string ChangeTournamentWebNameAction::getDescription() const {
    return "Change tournament web name";
}

