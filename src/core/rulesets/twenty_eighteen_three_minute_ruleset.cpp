#include "core/rulesets/twenty_eighteen_three_minute_ruleset.hpp"

std::unique_ptr<Ruleset> TwentyEighteenThreeMinuteRuleset::clone() const {
    return std::make_unique<TwentyEighteenThreeMinuteRuleset>();
}

std::string TwentyEighteenThreeMinuteRuleset::getName() const {
    return "Current (3 minutes)";
}

std::chrono::milliseconds TwentyEighteenThreeMinuteRuleset::getNormalTime() const {
    return std::chrono::minutes(3);
}

std::chrono::milliseconds TwentyEighteenThreeMinuteRuleset::getExpectedTime() const {
    return std::chrono::minutes(3);
}

RulesetIdentifier TwentyEighteenThreeMinuteRuleset::getIdentifier() const {
    return RulesetIdentifier::TWENTY_EIGHTEEN_THREE_MINUTE;
}

