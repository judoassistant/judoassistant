#include "actions/create_player_action.hpp"
#include "stores/tournament_store.hpp"

CreatePlayerAction::CreatePlayerAction(TournamentStore * store, const std::string & firstName, const std::string & lastName, uint8_t age)
    : mFirstName(firstName)
    , mLastName(lastName)
    , mAge(age)
    , mId(store->generateNextPlayerId())
{
}

bool CreatePlayerAction::operator()(TournamentStore * store) const {
    auto player = std::make_unique<PlayerStore>(mId, mFirstName, mLastName, mAge);
    store->addPlayer(std::move(player));
    return true;
}

std::unique_ptr<Action> CreatePlayerAction::getInverse() const {
    // TODO
}
