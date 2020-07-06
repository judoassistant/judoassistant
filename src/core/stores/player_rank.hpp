#pragma once

#include "core/core.hpp"
#include <vector>

class PlayerRank {
public:
    static const size_t SIZE = 16;
    enum Enum {
        KYU_6,
        KYU_5,
        KYU_4,
        KYU_3,
        KYU_2,
        KYU_1,
        DAN_1,
        DAN_2,
        DAN_3,
        DAN_4,
        DAN_5,
        DAN_6,
        DAN_7,
        DAN_8,
        DAN_9,
        DAN_10,
    };

    PlayerRank() {}
    PlayerRank(const PlayerRank &other) = default;
    PlayerRank(const std::string &str);
    PlayerRank(int value);
    PlayerRank(Enum value) : mValue(value) {}

    std::string toString() const;
    std::vector<std::string> strings() const; // Returns a vector of all possible names for this rank
    int toInt() const;
    static std::vector<PlayerRank> values();

    bool operator==(const PlayerRank &other) const {
        return mValue == other.mValue;
    }

    bool operator!=(const PlayerRank &other) const {
        return mValue != other.mValue;
    }

    bool operator<(const PlayerRank &other) const {
        return mValue < other.mValue;
    }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(mValue);
    }

private:
    Enum mValue;
    static std::string stringToLower(const std::string &str); // TODO: figure out how to properly handle unicode strings
};

std::ostream & operator<<(std::ostream &out, const PlayerRank &rank);

