#pragma once

#include <vector>
#include <QString>

#include "core/core.hpp"

/*
 * Given two strings compare them such that sequential digits in the
 * string are treated as a combined single number. For example, this results
 * in the string "MA70" comparing smaller than "MA100"
 */
class NumericalStringComparator {
private:
    std::vector<std::pair<bool, QString>> split_string(const QString &str) const;

public:
    bool operator()(const QString &a, const QString &b) const;
};
