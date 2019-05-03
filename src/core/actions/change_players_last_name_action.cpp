#include "core/actions/change_players_last_name_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersLastNameAction::ChangePlayersLastNameAction(std::vector<PlayerId> playerIds, const std::string &value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersLastNameAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getLastName());
        player.setLastName(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersLastNameAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setLastName(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

std::string ChangePlayersLastNameAction::getDescription() const {
    return "Change players last name";
}

std::unique_ptr<Action> ChangePlayersLastNameAction::freshClone() const {
    return std::make_unique<ChangePlayersLastNameAction>(mPlayerIds, mValue);
}
