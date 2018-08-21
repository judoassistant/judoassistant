#include "actions/create_player_action.hpp"
#include "stores/tournament_store.hpp"
#include "exception.hpp"

CreatePlayerAction::CreatePlayerAction(std::unique_ptr<TournamentStore> & tournament, const std::string & firstName, const std::string & lastName, uint8_t age)
    : mFirstName(firstName)
    , mLastName(lastName)
    , mAge(age)
    , mId(tournament->generateNextPlayerId())
{
}

bool CreatePlayerAction::operator()(std::unique_ptr<TournamentStore> & tournament) const {
    auto player = std::make_unique<PlayerStore>(mId, mFirstName, mLastName, mAge);
    tournament->addPlayer(std::move(player));
    tournament->playerAdded(mId);
    return true;
}

std::unique_ptr<Action> CreatePlayerAction::getInverse() const {
    throw new NotImplementedException;
    // TODO: Implement method
}
