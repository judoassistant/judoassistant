#pragma once

#include <vector>
#include "core/core.hpp"

class PlayerCountry {
public:
    static const size_t SIZE = 3;
    enum Enum {
        DENMARK,
        GREAT_BRITAIN,
        FRANCE,
    };

    PlayerCountry() {}
    PlayerCountry(const PlayerCountry &other) = default;
    PlayerCountry(const std::string &str);
    PlayerCountry(int value);
    PlayerCountry(Enum value) : mValue(value) {}

    std::string toString() const;
    std::string countryCode() const; // Returns the 3-letter IOC country code
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

private:
    Enum mValue;
};

std::ostream & operator<<(std::ostream &out, const PlayerCountry &country);

