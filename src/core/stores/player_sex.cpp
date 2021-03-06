#include "core/stores/player_sex.hpp"

PlayerSex::PlayerSex(int value)
    : mValue(static_cast<Enum>(value))
{
    if (mValue < 0 || mValue > 1)
        throw std::invalid_argument(std::to_string(value));
}

std::string PlayerSex::toString() const {
    switch (mValue) {
        case MALE: return "MALE";
        case FEMALE: return "FEMALE";
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

