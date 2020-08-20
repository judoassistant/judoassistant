#include "ui/misc/numerical_string_comparator.hpp"

std::vector<std::pair<bool, QString>> NumericalStringComparator::split_string(const QString &str) const {
    std::vector<std::pair<bool, QString>> res;
    QString current;
    bool isNumber = false; // Set to false to suppress compiler uninitialized warnings

    for (QChar a : str) {
        if (!current.isEmpty()) {
            if (a.isDigit() != isNumber) {
                res.push_back(std::make_pair(isNumber, current));
                current.clear();
            }
        }

        isNumber = a.isDigit();
        current.append(a);
    }

    if (!current.isEmpty())
        res.push_back(std::make_pair(isNumber, current));

    return res;
}

bool NumericalStringComparator::operator()(const QString &a, const QString &b) const {
    auto splitA = split_string(a);
    auto splitB = split_string(b);

    auto itA = splitA.begin();
    auto itB = splitB.begin();

    // compare the parts lexicographically
    for (; itA != splitA.end() && itB != splitB.end(); ++itA, ++itB) {
        const QString &partA = itA->second;
        const QString &partB = itB->second;

        if (partA == partB)
            continue;

        if (itA->first && itB->first) { // Both parts are number
            int numberA = partA.toInt();
            int numberB = partB.toInt();

            if (numberA != numberB)
                return numberA < numberB;
            continue;
        }

        return partA < partB;
    }

    return splitA.size() < splitB.size(); // Use size as tie breaker
}

