#pragma once

#include <vector>

#include "core.hpp"
#include "actions/action.hpp"

class CompositeAction : public Action {
public:
    CompositeAction(std::vector<std::unique_ptr<Action>> && actions);
private:
    std::vector<std::unique_ptr<Action>> mActions;
};

