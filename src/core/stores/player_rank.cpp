#include "core/stores/player_rank.hpp"

int PlayerRank::toInt() const {
    return static_cast<int>(mValue);
}

std::string PlayerRank::toString() const {
    switch (mValue) {
        case KYU_6: return "KYU_6";
        case KYU_5: return "KYU_5";
        case KYU_4: return "KYU_4";
        case KYU_3: return "KYU_3";
        case KYU_2: return "KYU_2";
        case KYU_1: return "KYU_1";
        case DAN_1: return "DAN_1";
        case DAN_2: return "DAN_2";
        case DAN_3: return "DAN_3";
        case DAN_4: return "DAN_4";
        case DAN_5: return "DAN_5";
        case DAN_6: return "DAN_6";
        case DAN_7: return "DAN_7";
        case DAN_8: return "DAN_8";
        case DAN_9: return "DAN_9";
        case DAN_10: return "DAN_10";
        default: return "";
    }
}

std::vector<PlayerRank> PlayerRank::values() {
    std::vector<PlayerRank> res;
    for (size_t i = 0; i < SIZE; ++i) {
        res.push_back(PlayerRank(static_cast<Enum>(i)));
    }

    return res;
}

PlayerRank::PlayerRank(int value) {
    if (value < 0 || value >= static_cast<int>(SIZE))
        throw std::invalid_argument(std::to_string(value));

    mValue = static_cast<Enum>(value);
}

std::ostream & operator<<(std::ostream &out, const PlayerRank &rank) {
    return out << rank.toString();
}

