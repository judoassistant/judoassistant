#pragma once

#include <vector>

#include "core.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"

class CompositeAction : public Action {
public:
    CompositeAction() = default;
    CompositeAction(std::vector<std::unique_ptr<Action>> actions);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mActions);
    }

private:
    std::vector<std::unique_ptr<Action>> mActions;
};

CEREAL_REGISTER_TYPE(CompositeAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, CompositeAction)
