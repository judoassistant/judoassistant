#include "core/stores/player_age.hpp"

PlayerAge::PlayerAge(int value)
    : mValue(value)
{
    if (mValue < min() || mValue > max())
        throw std::invalid_argument(std::to_string(value));
}

std::string PlayerAge::toString() const {
    return std::to_string(mValue);
}

int PlayerAge::toInt() const {
    return mValue;
}

std::ostream & operator<<(std::ostream &out, const PlayerAge &age) {
    return out << age.toString();
}

