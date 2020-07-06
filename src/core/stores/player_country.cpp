#include <cctype>

#include "core/stores/player_country.hpp"

std::string PlayerCountry::stringToLower(const std::string &str) {
    std::string res;
    for (unsigned char c : str) {
        res += std::tolower(c);
    }
    return res;
}

PlayerCountry::PlayerCountry(const std::string &str) {
    for (int i = 0; i < static_cast<int>(SIZE); ++i) {
        auto country = PlayerCountry(i);

        const std::string lower = stringToLower(str);
        for (const std::string &repr : country.strings()) {
            if (stringToLower(repr) == lower) {
                mValue = static_cast<Enum>(i);
                return;
            }
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
        case FRANCE: return "France";
        default: return "";
    }
}

std::string PlayerCountry::countryCode() const {
    switch (mValue) {
        case DENMARK: return "DEN";
        case FRANCE: return "FRA";
        default: return "";
    }
}

std::vector<std::string> PlayerCountry::strings() const {
    switch (mValue) {
        case DENMARK: return {"Denmark", "DEN", "DK", "Danmark"};
        case FRANCE: return {"France", "FRA", "FR"};
        default: return {};
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

