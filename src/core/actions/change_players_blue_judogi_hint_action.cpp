#include "core/actions/change_players_blue_judogi_hint_action.hpp"
#include "core/stores/tournament_store.hpp"

ChangePlayersBlueJudogiHintAction::ChangePlayersBlueJudogiHintAction(std::vector<PlayerId> playerIds, bool value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersBlueJudogiHintAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        if (player.getBlueJudogiHint() == mValue)
            continue;
        mChangedPlayers.push_back(playerId);
        mOldValues.push_back(player.getBlueJudogiHint());
        player.setBlueJudogiHint(mValue);
    }
    tournament.changePlayers(mChangedPlayers);
}

void ChangePlayersBlueJudogiHintAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mChangedPlayers) {
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setBlueJudogiHint(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mChangedPlayers);
    mChangedPlayers.clear();
    mOldValues.clear();
}

std::string ChangePlayersBlueJudogiHintAction::getDescription() const {
    return "Change players blue judogi hint";
}

std::unique_ptr<Action> ChangePlayersBlueJudogiHintAction::freshClone() const {
    return std::make_unique<ChangePlayersBlueJudogiHintAction>(mPlayerIds, mValue);
}

