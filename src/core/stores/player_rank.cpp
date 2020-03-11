#include "core/stores/player_rank.hpp"

int PlayerRank::toInt() const {
    return static_cast<int>(mValue);
}

std::string PlayerRank::toString() const {
    switch (mValue) {
        case KYU_6: return "6th kyu";
        case KYU_5: return "5th kyu";
        case KYU_4: return "4th kyu";
        case KYU_3: return "3rd kyu";
        case KYU_2: return "2nd kyu";
        case KYU_1: return "1st kyu";
        case DAN_1: return "1st dan";
        case DAN_2: return "2nd dan";
        case DAN_3: return "3rd dan";
        case DAN_4: return "4th dan";
        case DAN_5: return "5th dan";
        case DAN_6: return "6th dan";
        case DAN_7: return "7th dan";
        case DAN_8: return "8th dan";
        case DAN_9: return "9th dan";
        case DAN_10: return "10th dan";
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

PlayerRank::PlayerRank(const std::string &str) {
    for (int i = 0; i < static_cast<int>(SIZE); ++i) {
        auto rank = PlayerRank(i);
        if (rank.toString() == str) {
            mValue = static_cast<Enum>(i);
            return;
        }
    }

    throw std::invalid_argument(str);
}

PlayerRank::PlayerRank(int value) {
    if (value < 0 || value >= static_cast<int>(SIZE))
        throw std::invalid_argument(std::to_string(value));

    mValue = static_cast<Enum>(value);
}

std::ostream & operator<<(std::ostream &out, const PlayerRank &rank) {
    return out << rank.toString();
}

