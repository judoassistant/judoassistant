#include "actions/create_player_action.hpp"
#include "stores/tournament_store.hpp"

CreatePlayerAction::CreatePlayerAction(TournamentStore * store, std::string firstName, std::string lastName, uint8_t age) 
    : mFirstName(firstName)
    , mLastName(lastName)
    , mAge(age)
    , mId(store->mNextPlayerId++)
{
}

virtual void CreatePlayerAction::redo(TournamentStore * store) {
    // TODO
}

virtual void CreatePlayerAction::undo(TournamentStore * store) {
    // TODO
}
