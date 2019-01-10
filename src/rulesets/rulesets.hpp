#pragma once

#include "rulesets/twenty_eighteen_ruleset.hpp"
#include "rulesets/twenty_eighteen_children_ruleset.hpp"
#include "rulesets/twenty_eighteen_veteran_ruleset.hpp"

class Rulesets {
public:
    static const std::vector<std::unique_ptr<Ruleset>> & getRulesets();
private:
};
