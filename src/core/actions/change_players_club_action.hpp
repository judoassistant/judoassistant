#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"
#include "core/stores/player_store.hpp"

class ChangePlayersClubAction : public Action {
public:
    ChangePlayersClubAction() = default;
    ChangePlayersClubAction(std::vector<PlayerId> playerIds, const std::string &value);
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
    std::string mValue;

    // undo members
    std::vector<PlayerId> mChangedPlayers;
    std::vector<std::string> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersClubAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersClubAction)

