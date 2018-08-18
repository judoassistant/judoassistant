#include "actions/composite_action.hpp"

CompositeAction::CompositeAction(std::vector<std::unique_ptr<Action>> && actions)
    : mActions(std::move(actions))
{}
