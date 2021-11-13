#include "ui/stores/qplayer_age.hpp"
#include "core/log.hpp"
#include <QDate>
#include <QRegExp>
#include <QLocale>

// Try to parse a string to float using C locale and system locale
int stringToInt(const QString &str, bool *ok) {
    // Parse using system locale
    QLocale systemLocale;
    float value = systemLocale.toFloat(str, ok);
    if (*ok)
        return value;

    // Fallback to cLocale
    QLocale cLocale(QLocale::C);
    return cLocale.toFloat(str, ok);
}

// Parse a string to a date
QDate stringToDate(const QString &str) {
    QDate date = QDate::fromString(str.trimmed(), "dd-MM-yyyy");
    if (!date.isValid())
        date = QDate::fromString(str.trimmed(), "yyyy-MM-dd");
    return date;
}

// Compute the number of full years between from and to
int getYearsPassed(QDate from, QDate to) {
    if (from > to)
        return -getYearsPassed(to, from);

    // We can assume to happens after from
    const int years = to.year() - from.year();

    if (to.month() > from.month() || (to.month() == from.month() && to.day() >= from.day()))
        return years;
    return years - 1;
}

// Try to parse a string as a date and compute years since
int dateStringToInt(const QString &str, bool *ok) {
    // Attempt to parse date
    const QDate date = stringToDate(str);

    if (!date.isValid()) {
        *ok = false;
        return 0;
    }

    // Compute age as years passed
    const QDate now = QDate::currentDate();
    const int age = getYearsPassed(date, now);
    *ok = true;

    return age;
}

QPlayerAge QPlayerAge::fromHumanString(const QString &str) {
    bool ok;
    int value = stringToInt(str, &ok);

    if (!ok)
        value = dateStringToInt(str, &ok);

    if(!ok || value < min() || value > max())
        throw std::invalid_argument(str.toStdString());

    return QPlayerAge(value);
}

QString QPlayerAge::toHumanString() const {
    return QString::number(mValue);
}

