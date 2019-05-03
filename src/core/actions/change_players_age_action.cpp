#include "core/actions/change_players_age_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersAgeAction::ChangePlayersAgeAction(std::vector<PlayerId> playerIds, std::optional<PlayerAge> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersAgeAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getAge());
        player.setAge(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersAgeAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setAge(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

std::unique_ptr<Action> ChangePlayersAgeAction::freshClone() const {
    return std::make_unique<ChangePlayersAgeAction>(mPlayerIds, mValue);
}

std::string ChangePlayersAgeAction::getDescription() const {
    return "Change players age";
}

