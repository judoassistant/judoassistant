#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"
#include "core/stores/player_store.hpp"

class ChangePlayersBlueJudogiHintAction : public Action {
public:
    ChangePlayersBlueJudogiHintAction() = default;
    ChangePlayersBlueJudogiHintAction(std::vector<PlayerId> playerIds, bool value);
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
    bool mValue;

    // undo members
    std::vector<PlayerId> mChangedPlayers;
    std::vector<bool> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersBlueJudogiHintAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersBlueJudogiHintAction)

