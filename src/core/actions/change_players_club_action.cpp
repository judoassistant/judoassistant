#include "core/actions/change_players_club_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersClubAction::ChangePlayersClubAction(std::vector<PlayerId> playerIds, const std::string &value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersClubAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (player.getClub() == mValue)
            continue;

        mChangedPlayers.push_back(playerId);
        mOldValues.push_back(player.getClub());
        player.setClub(mValue);
    }
    tournament.changePlayers(mChangedPlayers);
}

void ChangePlayersClubAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mChangedPlayers) {
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setClub(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mChangedPlayers);
    mChangedPlayers.clear();
    mOldValues.clear();
}

std::string ChangePlayersClubAction::getDescription() const {
    return "Change players club";
}

std::unique_ptr<Action> ChangePlayersClubAction::freshClone() const {
    return std::make_unique<ChangePlayersClubAction>(mPlayerIds, mValue);
}

