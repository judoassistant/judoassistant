#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"
#include "core/stores/player_store.hpp"

class ChangePlayersCountryAction : public Action {
public:
    ChangePlayersCountryAction() = default;
    ChangePlayersCountryAction(std::vector<PlayerId> playerIds, std::optional<PlayerCountry> value);
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
    std::optional<PlayerCountry> mValue;

    // undo members
    std::vector<std::optional<PlayerCountry>> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangePlayersCountryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersCountryAction)

