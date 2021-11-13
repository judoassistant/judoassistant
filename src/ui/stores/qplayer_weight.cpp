#include "core/log.hpp"
#include "ui/stores/qplayer_weight.hpp"
#include <QLocale>
#include <QRegExp>

QPlayerWeight QPlayerWeight::fromHumanString(const QString &str) {
    // Find part of string corresponding to a number
    QRegExp reg_exp("(\\d+([,\\.]\\d\\d?)?)");
    int res = reg_exp.indexIn(str);

    if (res == -1)
        throw std::invalid_argument(str.toStdString());

    QString match = reg_exp.cap(0);

    // Try to parse the string with C locale and system locale
    QLocale systemLocale;
    QLocale cLocale(QLocale::C);

    bool ok;
    float value = systemLocale.toFloat(match, &ok);

    if (!ok)
        value = cLocale.toFloat(match, &ok);

    if (!ok || value < min() || value > max())
        throw std::invalid_argument(str.toStdString());

    return QPlayerWeight(value);
}

QString QPlayerWeight::toHumanString() const {
    return QString::number(mValue);
}

