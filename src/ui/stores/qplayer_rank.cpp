#include "ui/stores/qplayer_rank.hpp"

std::vector<QString> QPlayerRank::strings() const {
    switch (mValue) {
        case KYU_6: return {tr("6th kyu"), tr("6. kyu"), tr("white"), tr("6 kyu")};
        case KYU_5: return {tr("5th kyu"), tr("5. kyu"), tr("yellow"), tr("5 kyu")};
        case KYU_4: return {tr("5th kyu"), tr("4. kyu"), tr("orange"), tr("4 kyu")};
        case KYU_3: return {tr("3rd kyu"), tr("3. kyu"), tr("green"), tr("3 kyu")};
        case KYU_2: return {tr("2nd kyu"), tr("2. kyu"), tr("blue"), tr("2 kyu")};
        case KYU_1: return {tr("1st kyu"), tr("1. kyu"), tr("brown"), tr("1 kyu")};
        case DAN_1: return {tr("1st dan"), tr("1. dan"), tr("black"), tr("1 dan")};
        case DAN_2: return {tr("2nd dan"), tr("2. dan"), tr("2 dan")};
        case DAN_3: return {tr("3rd dan"), tr("3. dan"), tr("3 dan")};
        case DAN_4: return {tr("4th dan"), tr("4. dan"), tr("4 dan")};
        case DAN_5: return {tr("5th dan"), tr("5. dan"), tr("5 dan")};
        case DAN_6: return {tr("6th dan"), tr("6. dan"), tr("6 dan")};
        case DAN_7: return {tr("7th dan"), tr("7. dan"), tr("7 dan")};
        case DAN_8: return {tr("8th dan"), tr("8. dan"), tr("8 dan")};
        case DAN_9: return {tr("9th dan"), tr("9. dan"), tr("9 dan")};
        case DAN_10: return {tr("10th dan"), tr("10. dan"), tr("10 dan")};
        default: return {};
    }
}

QPlayerRank QPlayerRank::fromHumanString(const QString &str) {
    for (int i = 0; i < static_cast<int>(SIZE); ++i) {
        QPlayerRank rank(i);

        const auto lower = str.toLower();
        for (const QString &repr : rank.strings()) {
            if (repr.toLower() == lower)
                return rank;
        }
    }

    throw std::invalid_argument(str.toStdString());
}

QString QPlayerRank::toHumanString() const {
    switch (mValue) {
        case KYU_6: return tr("6th kyu");
        case KYU_5: return tr("5th kyu");
        case KYU_4: return tr("4th kyu");
        case KYU_3: return tr("3rd kyu");
        case KYU_2: return tr("2nd kyu");
        case KYU_1: return tr("1st kyu");
        case DAN_1: return tr("1st dan");
        case DAN_2: return tr("2nd dan");
        case DAN_3: return tr("3rd dan");
        case DAN_4: return tr("4th dan");
        case DAN_5: return tr("5th dan");
        case DAN_6: return tr("6th dan");
        case DAN_7: return tr("7th dan");
        case DAN_8: return tr("8th dan");
        case DAN_9: return tr("9th dan");
        case DAN_10: return tr("10th dan");
        default: return "";
    }
}

