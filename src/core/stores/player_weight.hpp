#pragma once

#include "core/core.hpp"
#include "core/serialize.hpp"

class PlayerWeight {
public:
    PlayerWeight() {}
    PlayerWeight(const PlayerWeight &other) = default;
    PlayerWeight(float weight);
    PlayerWeight(const std::string &str);

    std::string toString() const;
    float toFloat() const;

    bool operator==(const PlayerWeight &other) const {
        return mValue == other.mValue;
    }

    bool operator!=(const PlayerWeight &other) const {
        return mValue != other.mValue;
    }

    bool operator<(const PlayerWeight &other) const {
        return mValue < other.mValue;
    }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(mValue);
    }

    static constexpr float max() {
        return 400;
    }

    static constexpr float min() {
        return 0;
    }

protected:
    float mValue;
};

std::ostream & operator<<(std::ostream &out, const PlayerWeight &weight);

