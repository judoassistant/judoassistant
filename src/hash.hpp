#pragma once

#include <unordered_set>

namespace std {
    template <typename A, typename B>
    struct hash<std::pair<A, B>> {
        size_t operator()(const std::pair<A, B> &v) const {
            return std::hash<A>()(v.first) ^ std::hash<B>()(v.second);
        }
    };
};
