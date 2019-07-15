#pragma once

#include <vector>
#include <string>

#include "core/core.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"
#include "ui/misc/numerical_string_comparator.hpp"

class TournamentStore;
class CategoryId;

/*
 * Given two categories compare their names using the numerical string
 * comparator
 */
class CategoryNameComparator {
public:
    CategoryNameComparator(const TournamentStore &tournament);
    bool operator()(const CategoryId &a, const CategoryId &b) const;

private:
    NumericalStringComparator mComp;
    const TournamentStore &mTournament;
};
