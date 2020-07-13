#include "ui/stores/qplayer_age.hpp"

QPlayerAge QPlayerAge::fromHumanString(const QString &str) {
    bool ok;
    float value = str.toInt(&ok);

    if (!ok || value < min() || value > max())
        throw std::invalid_argument(str.toStdString());

    return QPlayerAge(value);
}

QString QPlayerAge::toHumanString() const {
    return QString::number(mValue);
}

