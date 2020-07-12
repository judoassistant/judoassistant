#include "ui/stores/qplayer_weight.hpp"

QPlayerWeight QPlayerWeight::fromHumanString(const QString &str) {
    bool ok;
    float value;

    // Remove a prepended '-' symbol, which is often used for denoting weight classes
    if (!str.isEmpty() && str.front() == '-')
        value = str.right(str.size()-1).toFloat(&ok);
    else
        value = str.toFloat(&ok);

    if (!ok || value < min() || value > max())
        throw std::invalid_argument(str.toStdString());

    return QPlayerWeight(value);
}

QString QPlayerWeight::toHumanString() const {
    return QString::number(mValue);
}

