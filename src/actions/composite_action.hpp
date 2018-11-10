#pragma once

#include <vector>

#include "core.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"

class CompositeAction : public Action {
public:
    CompositeAction(std::vector<std::unique_ptr<Action>> actions);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
private:
    std::vector<std::unique_ptr<Action>> mActions;
};

