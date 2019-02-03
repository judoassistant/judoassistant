#pragma once

#include <vector>
#include <string>

#include "core/core.hpp"

/*
 * Given two strings compare them in a way such that numbers in the string are
 * compared as numbers. For example the string "MA70" should
 * compare smaller than "MA100"
 */
class NumericalStringComparator {
private:
    std::vector<std::pair<bool, std::string>> split_string(const std::string & str) const;

public:
    bool operator()(const std::string &a, const std::string &b) const;
};
