#include "ui/stores/qplayer_country.hpp"

std::vector<QString> QPlayerCountry::strings() const {
    switch (mValue) {
        case DENMARK: return {tr("Denmark"), tr("DEN"), tr("DK"), tr("Danmark")};
        case FRANCE: return {tr("France"), tr("FRA"), tr("FR")};
        default: return {};
    }
}

QPlayerCountry QPlayerCountry::fromHumanString(const QString &str) {
    for (int i = 0; i < static_cast<int>(SIZE); ++i) {
        auto country = QPlayerCountry(i);

        const QString lower = str.toLower();
        for (const QString &repr : country.strings()) {
            if (repr.toLower() == lower)
                return country;
        }
    }

    throw std::invalid_argument(str.toStdString());
}

QString QPlayerCountry::toHumanString() const {
    switch (mValue) {
        case DENMARK: return tr("Denmark");
        case FRANCE: return tr("France");
        default: return "";
    }
}

QString QPlayerCountry::countryCode() const {
    switch (mValue) {
        case DENMARK: return tr("DEN");
        case FRANCE: return tr("FRA");
        default: return "";
    }
}

