#include "core/rulesets/twenty_eighteen_children_ruleset.hpp"

std::unique_ptr<Ruleset> TwentyEighteenChildrenRuleset::clone() const {
    return std::make_unique<TwentyEighteenChildrenRuleset>();
}

std::string TwentyEighteenChildrenRuleset::getName() const {
    return "2018 Children";
}

std::chrono::milliseconds TwentyEighteenChildrenRuleset::getNormalTime() const {
    return std::chrono::minutes(3);
}

std::chrono::milliseconds TwentyEighteenChildrenRuleset::getExpectedTime() const {
    return std::chrono::minutes(3);
}

RulesetIdentifier TwentyEighteenChildrenRuleset::getIdentifier() const {
    return RulesetIdentifier::TWENTY_EIGHTEEN_CHILDREN;
}

