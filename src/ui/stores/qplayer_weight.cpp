#include "core/log.hpp"
#include "ui/stores/qplayer_weight.hpp"
#include <QLocale>
#include <QRegExp>

// Find part of string corresponding to a number
QString matchString(const QString &str) {
    QRegExp reg_exp("(\\d+([,\\.]\\d\\d?)?)");
    int res = reg_exp.indexIn(str);

    if (res == -1)
        throw std::invalid_argument(str.toStdString());

    return reg_exp.cap(0);
}

// Try to parse a string to float using C locale and system locale
float StringToFloat(const QString &str) {
    // Parse using system locale
    {
        QLocale systemLocale;
        bool ok;
        float value = systemLocale.toFloat(str, &ok);
        if (ok)
            return value;
    }

    {
        QLocale cLocale(QLocale::C);
        bool ok;
        float value = cLocale.toFloat(str, &ok);
        if (ok)
            return value;
    }

    throw std::invalid_argument(str.toStdString());

    return 0;
}

QPlayerWeight QPlayerWeight::fromHumanString(const QString &str) {
    const QString match = matchString(str);
    float value = StringToFloat(match);

    if (value < min() || value > max())
        throw std::invalid_argument(str.toStdString());

    return QPlayerWeight(value);
}

QString QPlayerWeight::toHumanString() const {
    return QString::number(mValue);
}

