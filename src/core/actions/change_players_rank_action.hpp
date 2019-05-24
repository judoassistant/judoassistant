#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"
#include "core/stores/player_store.hpp"

class ChangePlayersRankAction : public Action {
public:
    ChangePlayersRankAction() = default;
    ChangePlayersRankAction(std::vector<PlayerId> playerIds, std::optional<PlayerRank> value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
        ar(mValue);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    std::optional<PlayerRank> mValue;

    // undo members
    std::vector<PlayerId> mChangedPlayers;
    std::vector<std::optional<PlayerRank>> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersRankAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersRankAction)

