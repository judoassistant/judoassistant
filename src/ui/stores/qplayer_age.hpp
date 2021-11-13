#pragma once

#include <QString>

#include "core/stores/player_age.hpp"

class QPlayerAge : public PlayerAge {
public:
    QPlayerAge(const QPlayerAge &other): PlayerAge(other) {}
    QPlayerAge(const PlayerAge &other): PlayerAge(other) {}
    QPlayerAge(float weight): PlayerAge(weight) {}

    static QPlayerAge fromHumanString(const QString &str); // Interprets a string in the current locale as an age
    QString toHumanString() const; // Returns a string representing the age in the current locale
};

