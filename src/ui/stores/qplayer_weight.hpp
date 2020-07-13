#pragma once

#include <QString>

#include "core/stores/player_weight.hpp"

class QPlayerWeight : public PlayerWeight {
public:
    QPlayerWeight(const QPlayerWeight &other): PlayerWeight(other) {}
    QPlayerWeight(const PlayerWeight &other): PlayerWeight(other) {}
    QPlayerWeight(float weight): PlayerWeight(weight) {}

    static QPlayerWeight fromHumanString(const QString &str); // Interprets a string in the current locale as a weight
    QString toHumanString() const; // Returns a string representing the weight in the current locale
};
