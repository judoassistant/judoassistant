#include "core/rulesets/twenty_eighteen_two_minute_ruleset.hpp"

std::unique_ptr<Ruleset> TwentyEighteenTwoMinuteRuleset::clone() const {
    return std::make_unique<TwentyEighteenTwoMinuteRuleset>();
}

std::string TwentyEighteenTwoMinuteRuleset::getName() const {
    return "Current (2 minutes)";
}

std::chrono::milliseconds TwentyEighteenTwoMinuteRuleset::getNormalTime() const {
    return std::chrono::minutes(2);
}

std::chrono::milliseconds TwentyEighteenTwoMinuteRuleset::getExpectedTime() const {
    return std::chrono::minutes(2);
}

RulesetIdentifier TwentyEighteenTwoMinuteRuleset::getIdentifier() const {
    return RulesetIdentifier::TWENTY_EIGHTEEN_TWO_MINUTE;
}

