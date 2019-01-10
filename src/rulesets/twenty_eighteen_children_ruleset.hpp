#pragma once

#include <cereal/types/polymorphic.hpp>

#include "rulesets/twenty_eighteen_ruleset.hpp"
#include "serialize.hpp"

// The IJF children ruleset from January 2018
class TwentyEighteenChildrenRuleset : public TwentyEighteenRuleset {
private:
public:
    std::string getName() const override;

    std::chrono::milliseconds getNormalTime() const override;

    std::unique_ptr<Ruleset> clone() const override;
};

CEREAL_REGISTER_TYPE(TwentyEighteenChildrenRuleset)
CEREAL_REGISTER_POLYMORPHIC_RELATION(TwentyEighteenRuleset, TwentyEighteenChildrenRuleset)
