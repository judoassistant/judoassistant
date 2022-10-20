#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"
#include "core/stores/player_store.hpp"

class ChangePlayersWeightAction : public Action {
public:
    ChangePlayersWeightAction() = default;
    ChangePlayersWeightAction(std::vector<PlayerId> playerIds, std::optional<PlayerWeight> value);
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
    std::vector<CategoryId> getCategoriesThatChange(TournamentStore & tournament);

    std::vector<PlayerId> mPlayerIds;
    std::optional<PlayerWeight> mValue;

    // undo members
    std::vector<PlayerId> mChangedPlayers;
    std::vector<std::optional<PlayerWeight>> mOldValues;
    std::stack<std::unique_ptr<Action>> mDrawActions;
    std::vector<CategoryId> mChangedCategories;
};

CEREAL_REGISTER_TYPE(ChangePlayersWeightAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangePlayersWeightAction)

