#pragma once

#include "core/core.hpp"
#include <vector>

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
    PlayerSex(const std::string &str);
    PlayerSex(Enum value) : mValue(value) {}


    std::string toString() const;
    std::vector<std::string> strings() const; // Returns a vector of all possible names for this sex
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

private:
    Enum mValue;
    static std::string stringToLower(const std::string &str); // TODO: figure out how to properly handle unicode strings
};

std::ostream & operator<<(std::ostream &out, const PlayerSex &sex);

