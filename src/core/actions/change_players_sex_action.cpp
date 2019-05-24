#include "core/actions/change_players_sex_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersSexAction::ChangePlayersSexAction(std::vector<PlayerId> playerIds, std::optional<PlayerSex> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersSexAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (player.getSex() == mValue)
            continue;
        mChangedPlayers.push_back(playerId);
        mOldValues.push_back(player.getSex());
        player.setSex(mValue);
    }
    tournament.changePlayers(mChangedPlayers);
}

void ChangePlayersSexAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mChangedPlayers) {
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setSex(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mChangedPlayers);
    mChangedPlayers.clear();
    mOldValues.clear();
}

std::unique_ptr<Action> ChangePlayersSexAction::freshClone() const {
    return std::make_unique<ChangePlayersSexAction>(mPlayerIds, mValue);
}

std::string ChangePlayersSexAction::getDescription() const {
    return "Change players sex";
}

