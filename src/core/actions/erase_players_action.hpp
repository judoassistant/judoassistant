#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"
#include "core/stores/player_store.hpp"

class ErasePlayersFromCategoryAction;

class ErasePlayersAction : public Action {
public:
    ErasePlayersAction() = default;
    ErasePlayersAction(const std::vector<PlayerId> &playerIds);
    ErasePlayersAction(const std::vector<PlayerId> &playerIds, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds, mSeed);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    unsigned int mSeed;

    // undo members
    std::vector<PlayerId> mErasedPlayerIds;
    std::stack<std::unique_ptr<PlayerStore>> mPlayers;
    std::stack<std::unique_ptr<ErasePlayersFromCategoryAction>> mActions;
};

CEREAL_REGISTER_TYPE(ErasePlayersAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ErasePlayersAction)

