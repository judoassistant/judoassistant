#include "rulesets/rulesets.hpp"

const std::vector<std::unique_ptr<Ruleset>> & Rulesets::getRulesets() {
    static std::vector<std::unique_ptr<Ruleset>> rulesets;

    if (rulesets.empty()) {
        rulesets.push_back(std::make_unique<TwentyEighteenRuleset>());
    }

    return rulesets;
}
