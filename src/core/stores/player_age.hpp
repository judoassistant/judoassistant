#pragma once

#include "core/core.hpp"

class PlayerAge {
public:
    PlayerAge() {}
    PlayerAge(const PlayerAge &other) = default;
    PlayerAge(int age);
    PlayerAge(const std::string &str);

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

    static int max();
    static int min();

private:
    int mValue;
};

std::ostream & operator<<(std::ostream &out, const PlayerAge &age);

