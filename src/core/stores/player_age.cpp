#include "core/stores/player_age.hpp"

PlayerAge::PlayerAge(int value)
    : mValue(value)
{
    if (mValue < min() || mValue > max())
        throw std::invalid_argument(std::to_string(value));
}

PlayerAge::PlayerAge(const std::string &str) {
    try {
        mValue = std::stoi(str);
    }
    catch (const std::exception &e) {
        throw std::invalid_argument(str);
    }

    if (mValue < min() || mValue > max())
        throw std::invalid_argument(str);
}

std::string PlayerAge::toString() const {
    return std::to_string(mValue);
}

int PlayerAge::toInt() const {
    return mValue;
}

int PlayerAge::max() {
    return 255;
}

int PlayerAge::min() {
    return 0;
}

std::ostream & operator<<(std::ostream &out, const PlayerAge &age) {
    return out << age.toString();
}

