#include "core/actions/change_tournament_web_name_action.hpp"
#include "core/stores/tournament_store.hpp"

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

