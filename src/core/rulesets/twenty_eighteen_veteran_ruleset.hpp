#pragma once

#include <cereal/types/polymorphic.hpp>

#include "core/rulesets/twenty_eighteen_ruleset.hpp"
#include "core/serialize.hpp"

// The IJF children ruleset from January 2018
class TwentyEighteenVeteranRuleset : public TwentyEighteenRuleset {
private:
public:
    std::string getName() const override;
    RulesetIdentifier getIdentifier() const override;

    std::chrono::milliseconds getNormalTime() const override;
    std::chrono::milliseconds getExpectedTime() const override;

    std::unique_ptr<Ruleset> clone() const override;
};

CEREAL_REGISTER_TYPE(TwentyEighteenVeteranRuleset)
CEREAL_REGISTER_POLYMORPHIC_RELATION(TwentyEighteenRuleset, TwentyEighteenVeteranRuleset)
