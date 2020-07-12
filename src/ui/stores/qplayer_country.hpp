#pragma once

#include <QObject>
#include <QString>
#include "core/stores/player_country.hpp"

class QPlayerCountry : public QObject, public PlayerCountry {
    Q_OBJECT
public:
    QPlayerCountry(const QPlayerCountry &other): PlayerCountry(other) {}
    QPlayerCountry(const PlayerCountry &other): PlayerCountry(other) {}
    QPlayerCountry(int value): PlayerCountry(value) {}

    static QPlayerCountry fromHumanString(const QString &str); // Parse a string
    QString toHumanString() const; // Returns a string to display in the UI
    QString countryCode() const; // Returns the 3-letter IOC country code

private:
    std::vector<QString> strings() const; // Returns a vector of all possible names for this country
};

