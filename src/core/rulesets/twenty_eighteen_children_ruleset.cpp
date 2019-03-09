#include "core/rulesets/twenty_eighteen_children_ruleset.hpp"

std::unique_ptr<Ruleset> TwentyEighteenChildrenRuleset::clone() const {
    return std::make_unique<TwentyEighteenChildrenRuleset>();
}

std::string TwentyEighteenChildrenRuleset::getName() const {
    // TODO: Handle delayed translation of this
    return "2018 Children";
}

std::chrono::milliseconds TwentyEighteenChildrenRuleset::getNormalTime() const {
    return std::chrono::minutes(3);
}

std::chrono::milliseconds TwentyEighteenChildrenRuleset::getEstimatedTime() const {
    return std::chrono::minutes(3);
}


