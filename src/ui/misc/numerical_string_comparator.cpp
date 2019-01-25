#include "ui/misc/numerical_string_comparator.hpp"

std::vector<std::pair<bool, std::string>> NumericalStringComparator::split_string(const std::string & str) const {
    std::vector<std::pair<bool, std::string>> res;
    std::string current;
    bool isNumber;

    for (char a : str) {
        bool isDigit = ('0' <= a && a <= '9');

        if (!current.empty()) {
            if (isDigit != isNumber) {
                res.push_back(std::make_pair(isNumber, current));
                current.clear();
            }
        }

        isNumber = isDigit;
        current.push_back(a);
    }

    if (!current.empty())
        res.push_back(std::make_pair(isNumber, current));

    return res;
}

bool NumericalStringComparator::operator()(const std::string &a, const std::string &b) const {
    auto splitA = split_string(a);
    auto splitB = split_string(b);

    auto itA = splitA.begin();
    auto itB = splitB.begin();

    // compare the parts lexicographically
    for (; itA != splitA.end() && itB != splitB.end(); ++itA, ++itB) {
        if (itA->second == itB->second)
            continue;

        if (itA->first && itA->first) { // Both parts are number
            int numberA = std::stoi(itA->second);
            int numberB = std::stoi(itB->second);

            if (numberA != numberB)
                return numberA < numberB;
            continue;
        }

        return itA->second < itB->second;
    }

    return splitA.size() < splitB.size();
}

