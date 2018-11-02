#pragma once

#include <unordered_set>

template <class T>
void hash_combine(size_t &seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

namespace std {
    template <typename A, typename B>
    struct hash<std::pair<A, B>> {
        size_t operator()(const std::pair<A, B> &v) const {
            size_t seed = 0;
            hash_combine(seed, v.first);
            hash_combine(seed, v.second);
            return seed;
        }
    };
};

