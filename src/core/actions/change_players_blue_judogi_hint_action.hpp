#pragma once

#include "core/actions/action.hpp"
#include "core/actions/confirmable_action.hpp"
#include "core/core.hpp"
#include "core/stores/player_store.hpp"

class ChangePlayersBlueJudogiHintAction : public Action, public ConfirmableAction {
public:
    ChangePlayersBlueJudogiHintAction() = default;
    ChangePlayersBlueJudogiHintAction(std::vector<PlayerId> playerIds, bool value);
    ChangePlayersBlueJudogiHintAction(std::vector<PlayerId> playerIds, bool value, size_t seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds, mValue, mSeed);
    }

    bool doesRequireConfirmation(const TournamentStore &tournament) const override;

private:
    std::vector<PlayerId> mPlayerIds;
    bool mValue;
    size_t mSeed;

    // undo members
    std::vector<PlayerId> mChangedPlayers;
    std::vector<bool> mOldValues;
    std::unique_ptr<Action> mDrawAction;
};

CEREAL_REGISTER_TYPE(ChangePlayersBlueJudogiHintAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersBlueJudogiHintAction)

