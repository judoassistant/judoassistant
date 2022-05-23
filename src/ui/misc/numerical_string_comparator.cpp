#include "ui/misc/numerical_string_comparator.hpp"

std::vector<std::pair<bool, QString>> NumericalStringComparator::split_string(const QString &str) const {
    std::vector<std::pair<bool, QString>> res;

    for (int i = 0; i < str.size(); ++i) {
        QString cur = str[i];

        if (str[i].isDigit()) {
            while (i+1 < str.size() && str[i+1].isDigit()) {
                cur += str[++i];
            }
        }

        res.emplace_back(str[i].isDigit(), cur);
    }


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

