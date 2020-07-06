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

std::vector<std::string> PlayerRank::strings() const {
    switch (mValue) {
        case KYU_6: return {"6th kyu", "6. kyu", "white", "6 kyu"};
        case KYU_5: return {"5th kyu", "5. kyu", "yellow", "5 kyu"};
        case KYU_4: return {"5th kyu", "4. kyu", "orange", "4 kyu"};
        case KYU_3: return {"3rd kyu", "3. kyu", "green", "3 kyu"};
        case KYU_2: return {"2nd kyu", "2. kyu", "blue", "2 kyu"};
        case KYU_1: return {"1st kyu", "1. kyu", "brown", "1 kyu"};
        case DAN_1: return {"1st dan", "1. dan", "black", "1 dan"};
        case DAN_2: return {"2nd dan", "2. dan", "2 dan"};
        case DAN_3: return {"3rd dan", "3. dan", "3 dan"};
        case DAN_4: return {"4th dan", "4. dan", "4 dan"};
        case DAN_5: return {"5th dan", "5. dan", "5 dan"};
        case DAN_6: return {"6th dan", "6. dan", "6 dan"};
        case DAN_7: return {"7th dan", "7. dan", "7 dan"};
        case DAN_8: return {"8th dan", "8. dan", "8 dan"};
        case DAN_9: return {"9th dan", "9. dan", "9 dan"};
        case DAN_10: return {"10th dan", "10. dan", "10 dan"};
        default: return {};
    }
}

std::vector<PlayerRank> PlayerRank::values() {
    std::vector<PlayerRank> res;
    for (size_t i = 0; i < SIZE; ++i) {
        res.push_back(PlayerRank(static_cast<Enum>(i)));
    }

    return res;
}

std::string PlayerRank::stringToLower(const std::string &str) {
    std::string res;
    for (unsigned char c : str) {
        res += std::tolower(c);
    }
    return res;
}

PlayerRank::PlayerRank(const std::string &str) {
    for (int i = 0; i < static_cast<int>(SIZE); ++i) {
        auto rank = PlayerRank(i);

        const std::string lower = stringToLower(str);
        for (const std::string &repr : rank.strings()) {
            if (stringToLower(repr) == lower) {
                mValue = static_cast<Enum>(i);
                return;
            }
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

