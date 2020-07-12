#include "core/stores/player_weight.hpp"

PlayerWeight::PlayerWeight(float value)
    : mValue(value)
{
    if (mValue < min() || mValue > max())
        throw std::invalid_argument(std::to_string(value));
}

std::string PlayerWeight::toString() const {
    return std::to_string(mValue);
}

float PlayerWeight::toFloat() const {
    return mValue;
}

std::ostream & operator<<(std::ostream &out, const PlayerWeight &weight) {
    return out << weight.toString();
}

