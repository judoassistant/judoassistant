#pragma once

#include "core/rulesets/twenty_eighteen_ruleset.hpp"
#include "core/rulesets/twenty_eighteen_children_ruleset.hpp"
#include "core/rulesets/twenty_eighteen_veteran_ruleset.hpp"

class Rulesets {
public:
    static const std::vector<std::unique_ptr<Ruleset>> & getRulesets();
private:
};
