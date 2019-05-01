#include "core/actions/change_players_country_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersCountryAction::ChangePlayersCountryAction(std::vector<PlayerId> playerIds, std::optional<PlayerCountry> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersCountryAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getCountry());
        player.setCountry(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersCountryAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setCountry(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

std::unique_ptr<Action> ChangePlayersCountryAction::freshClone() const {
    return std::make_unique<ChangePlayersCountryAction>(mPlayerIds, mValue);
}

std::string ChangePlayersCountryAction::getDescription() const {
    return "Change players country";
}

