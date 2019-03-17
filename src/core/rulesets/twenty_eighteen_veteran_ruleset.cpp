#include "core/rulesets/twenty_eighteen_veteran_ruleset.hpp"

std::unique_ptr<Ruleset> TwentyEighteenVeteranRuleset::clone() const {
    return std::make_unique<TwentyEighteenVeteranRuleset>();
}

std::string TwentyEighteenVeteranRuleset::getName() const {
    return "2018 Veteran";
}

std::chrono::milliseconds TwentyEighteenVeteranRuleset::getNormalTime() const {
    return std::chrono::minutes(3);
}

std::chrono::milliseconds TwentyEighteenVeteranRuleset::getExpectedTime() const {
    return std::chrono::minutes(4);
}

