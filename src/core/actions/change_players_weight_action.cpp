#include "core/actions/change_players_weight_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersWeightAction::ChangePlayersWeightAction(std::vector<PlayerId> playerIds, std::optional<PlayerWeight> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersWeightAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getWeight());
        player.setWeight(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersWeightAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setWeight(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

std::string ChangePlayersWeightAction::getDescription() const {
    return "Change players weight";
}

std::unique_ptr<Action> ChangePlayersWeightAction::freshClone() const {
    return std::make_unique<ChangePlayersWeightAction>(mPlayerIds, mValue);
}
