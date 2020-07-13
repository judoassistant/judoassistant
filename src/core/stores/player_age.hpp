#pragma once

#include "core/core.hpp"
#include "core/serialize.hpp"

class PlayerAge {
public:
    PlayerAge() {}
    PlayerAge(const PlayerAge &other) = default;
    PlayerAge(int age);

    std::string toString() const;
    int toInt() const;

    bool operator==(const PlayerAge &other) const {
        return mValue == other.mValue;
    }

    bool operator!=(const PlayerAge &other) const {
        return mValue != other.mValue;
    }

    bool operator<(const PlayerAge &other) const {
        return mValue < other.mValue;
    }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(mValue);
    }

    static constexpr int max() {
        return 255;
    }

    static constexpr int min() {
        return 0;
    }

protected:
    int mValue;
};

std::ostream & operator<<(std::ostream &out, const PlayerAge &age);

