#include <cctype>

#include "core/stores/player_country.hpp"


PlayerCountry::PlayerCountry(int value)
    : mValue(static_cast<Enum>(value))
{
    if (mValue < 0 || mValue >= SIZE)
        throw std::invalid_argument(std::to_string(value));
}

std::string PlayerCountry::toString() const {
    switch (mValue) {
        case DENMARK: return "DENMARK";
        case FRANCE: return "FRANCE";
        default: return "";
    }
}

std::vector<PlayerCountry> PlayerCountry::values() {
    std::vector<PlayerCountry> res;
    for (size_t i = 0; i < SIZE; ++i) {
        res.push_back(PlayerCountry(static_cast<Enum>(i)));
    }

    return res;
}

int PlayerCountry::toInt() const {
    return static_cast<int>(mValue);
}

std::ostream & operator<<(std::ostream &out, const PlayerCountry &country) {
    return out << country.toString();
}

