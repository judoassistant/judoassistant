#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"
#include "core/stores/player_store.hpp"

class ChangePlayersSexAction : public Action {
public:
    ChangePlayersSexAction() = default;
    ChangePlayersSexAction(std::vector<PlayerId> playerId, std::optional<PlayerSex> value);
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
    std::optional<PlayerSex> mValue;

    // undo members
    std::vector<std::optional<PlayerSex>> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersSexAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersSexAction)

