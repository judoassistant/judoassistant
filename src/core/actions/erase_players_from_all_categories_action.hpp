#pragma once

#include <stack>

#include "core/actions/action.hpp"
#include "core/actions/confirmable_action.hpp"

class TournamentStore;
class PlayerId;

class ErasePlayersFromAllCategoriesAction : public Action, public ConfirmableAction {
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

    bool doesRequireConfirmation(const TournamentStore &tournament) const override;

private:
    std::vector<PlayerId> mPlayerIds;
    unsigned int mSeed;

    // undo members
    std::stack<std::unique_ptr<Action>> mActions;
};

CEREAL_REGISTER_TYPE(ErasePlayersFromAllCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ErasePlayersFromAllCategoriesAction)

