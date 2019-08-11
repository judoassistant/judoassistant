#pragma once

#include <stack>

#include "core/actions/action.hpp"

class TournamentStore;
class PlayerId;
class ErasePlayersFromCategoryAction;

class ErasePlayersFromAllCategoriesAction : public Action {
public:
    ErasePlayersFromAllCategoriesAction() = default;
    ErasePlayersFromAllCategoriesAction(const std::vector<PlayerId> &playerIds);
    ErasePlayersFromAllCategoriesAction(const std::vector<PlayerId> &playerIds, unsigned int seed);
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
    std::stack<std::unique_ptr<ErasePlayersFromCategoryAction>> mActions;
};

CEREAL_REGISTER_TYPE(ErasePlayersFromAllCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ErasePlayersFromAllCategoriesAction)

