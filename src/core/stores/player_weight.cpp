#include "core/stores/player_weight.hpp"

PlayerWeight::PlayerWeight(float value)
    : mValue(value)
{
    if (mValue < min() || mValue > max())
        throw std::invalid_argument(std::to_string(value));
}

PlayerWeight::PlayerWeight(const std::string &str) {
    try {
        if (!str.empty() && str.front() == '-')
            mValue = std::stof(str.substr(1, str.size()-1));
        mValue = std::stof(str);
    }
    catch (const std::exception &e) {
        throw std::invalid_argument(str);
    }

    if (mValue < min() || mValue > max())
        throw std::invalid_argument(str);
}

std::string PlayerWeight::toString() const {
    return std::to_string(mValue);
}

float PlayerWeight::toFloat() const {
    return mValue;
}

float PlayerWeight::max() {
    return 400;
}

float PlayerWeight::min() {
    return 0;
}

std::ostream & operator<<(std::ostream &out, const PlayerWeight &weight) {
    return out << weight.toString();
}

