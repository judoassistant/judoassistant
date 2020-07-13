#pragma once

#include <vector>
#include <QString>

#include "core/core.hpp"

/*
 * Given two strings compare them in a way such that numbers in the string are
 * compared as numbers. For example the string "MA70" should
 * compare smaller than "MA100"
 */
class NumericalStringComparator {
private:
    std::vector<std::pair<bool, QString>> split_string(const QString &str) const;

public:
    bool operator()(const QString &a, const QString &b) const;
};
