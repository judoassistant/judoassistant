#include "core/actions/change_players_rank_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersRankAction::ChangePlayersRankAction(std::vector<PlayerId> playerIds, std::optional<PlayerRank> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersRankAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (player.getRank() == mValue)
            continue;
        mChangedPlayers.push_back(playerId);
        mOldValues.push_back(player.getRank());
        player.setRank(mValue);
    }
    tournament.changePlayers(mChangedPlayers);
}

void ChangePlayersRankAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mChangedPlayers) {
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setRank(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mChangedPlayers);
    mChangedPlayers.clear();
    mOldValues.clear();
}

std::unique_ptr<Action> ChangePlayersRankAction::freshClone() const {
    return std::make_unique<ChangePlayersRankAction>(mPlayerIds, mValue);
}

std::string ChangePlayersRankAction::getDescription() const {
    return "Change players rank";
}

