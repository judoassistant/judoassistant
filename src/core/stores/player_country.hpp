#pragma once

#include <vector>

#include "core/core.hpp"
#include "core/serialize.hpp"

class PlayerCountry {
public:
    static const size_t SIZE = 2;
    enum Enum {
        DENMARK,
        FRANCE,
    };

    PlayerCountry() {}
    PlayerCountry(const PlayerCountry &other) = default;
    PlayerCountry(int value);
    PlayerCountry(Enum value) : mValue(value) {}

    std::string toString() const;
    int toInt() const;
    static std::vector<PlayerCountry> values();

    bool operator==(const PlayerCountry &other) const {
        return mValue == other.mValue;
    }

    bool operator!=(const PlayerCountry &other) const {
        return mValue != other.mValue;
    }

    bool operator<(const PlayerCountry &other) const {
        return mValue < other.mValue;
    }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(mValue);
    }

protected:
    Enum mValue;
};

std::ostream & operator<<(std::ostream &out, const PlayerCountry &country);

