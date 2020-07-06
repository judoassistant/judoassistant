#include "core/stores/player_sex.hpp"

PlayerSex::PlayerSex(int value)
    : mValue(static_cast<Enum>(value))
{
    if (mValue < 0 || mValue > 1)
        throw std::invalid_argument(std::to_string(value));
}

PlayerSex::PlayerSex(const std::string &str) {
    for (int i = 0; i < static_cast<int>(SIZE); ++i) {
        auto sex = PlayerSex(i);

        const std::string lower = stringToLower(str);
        for (const std::string &repr : sex.strings()) {
            if (stringToLower(repr) == lower) {
                mValue = static_cast<Enum>(i);
                return;
            }
        }
    }

    throw std::invalid_argument(str);
}

std::string PlayerSex::toString() const {
    switch (mValue) {
        case MALE: return "Male";
        case FEMALE: return "Female";
        default: return "";
    }
}

std::vector<PlayerSex> PlayerSex::values() {
    std::vector<PlayerSex> res;
    for (size_t i = 0; i < SIZE; ++i) {
        res.push_back(PlayerSex(static_cast<Enum>(i)));
    }

    return res;
}

int PlayerSex::toInt() const {
    return static_cast<int>(mValue);
}

std::ostream & operator<<(std::ostream &out, const PlayerSex &sex) {
    return out << sex.toString();
}

std::string PlayerSex::stringToLower(const std::string &str) {
    std::string res;
    for (unsigned char c : str) {
        res += std::tolower(c);
    }
    return res;
}

std::vector<std::string> PlayerSex::strings() const {
    switch (mValue) {
        case MALE: return {"male", "man", "m", "boy"};
        case FEMALE: return {"female", "woman", "w", "f", "girl"};
        default: return {};
    }
}
