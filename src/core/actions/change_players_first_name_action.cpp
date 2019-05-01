#include "core/actions/change_players_first_name_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersFirstNameAction::ChangePlayersFirstNameAction(std::vector<PlayerId> playerIds, const std::string &value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

std::unique_ptr<Action> ChangePlayersFirstNameAction::freshClone() const {
    return std::make_unique<ChangePlayersFirstNameAction>(mPlayerIds, mValue);
}

void ChangePlayersFirstNameAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getFirstName());
        player.setFirstName(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersFirstNameAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setFirstName(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

std::string ChangePlayersFirstNameAction::getDescription() const {
    return "Change players first name";
}

