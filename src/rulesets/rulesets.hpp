#pragma once

#include "rulesets/twentyeighteen_ruleset.hpp"

class Rulesets {
public:
    static const std::vector<std::unique_ptr<Ruleset>> & getRulesets();
private:
};
