#pragma once

#include <random>

// std::shuffle is not consistent across platforms
template <class RandomIt>
void shuffle(RandomIt begin, RandomIt end, std::mt19937 &eng) {
    auto d = std::distance(begin, end);

    if (d <= 1)
        return;

    for (auto i = begin; i != end; ++i) {
        auto j  = begin + (eng() % d);

        if (i != j)
            std::swap(*i, *j);
    }
}

