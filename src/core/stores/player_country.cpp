#include "core/stores/player_country.hpp"

PlayerCountry::PlayerCountry(const std::string &str) {
    for (int i = 0; i < static_cast<int>(SIZE); ++i) {
        auto country = PlayerCountry(i);
        if (country.toString() == str) {
            mValue = static_cast<Enum>(i);
            return;
        }
    }

    throw std::invalid_argument(str);
}

PlayerCountry::PlayerCountry(int value)
    : mValue(static_cast<Enum>(value))
{
    if (mValue < 0 || mValue >= SIZE)
        throw std::invalid_argument(std::to_string(value));
}

std::string PlayerCountry::toString() const {
    switch (mValue) {
        case DENMARK: return "Denmark";
        case GREAT_BRITAIN: return "Great Britain";
        case FRANCE: return "France";
        default: return "";
    }
}

std::string PlayerCountry::countryCode() const {
    switch (mValue) {
        case DENMARK: return "DEN";
        case GREAT_BRITAIN: return "GBR";
        case FRANCE: return "FRA";
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

