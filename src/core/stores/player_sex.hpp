#pragma once

#include <vector>

#include "core/core.hpp"
#include "core/serialize.hpp"

class PlayerSex {
public:
    enum Enum {
        MALE,
        FEMALE,
    };
    static const size_t SIZE = 2;

    PlayerSex() {}
    PlayerSex(const PlayerSex &other) = default;
    PlayerSex(int value);
    PlayerSex(Enum value) : mValue(value) {}

    std::string toString() const; // Returns the enum represented as a string
    int toInt() const;
    static std::vector<PlayerSex> values();

    bool operator==(const PlayerSex &other) const {
        return mValue == other.mValue;
    }

    bool operator!=(const PlayerSex &other) const {
        return mValue != other.mValue;
    }

    bool operator<(const PlayerSex &other) const {
        return mValue < other.mValue;
    }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(mValue);
    }

protected:
    Enum mValue;
};

std::ostream & operator<<(std::ostream &out, const PlayerSex &sex);

