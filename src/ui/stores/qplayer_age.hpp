#pragma once

#include <QObject>

#include "core/stores/player_age.hpp"

class QPlayerAge : public QObject, public PlayerAge {
    Q_OBJECT
public:
    QPlayerAge(const QPlayerAge &other): PlayerAge(other) {}
    QPlayerAge(const PlayerAge &other): PlayerAge(other) {}
    QPlayerAge(float weight): PlayerAge(weight) {}

    static QPlayerAge fromHumanString(const QString &str); // Interprets a string in the current locale as a weight
    QString toHumanString() const; // Returns a string representing the weight in the current locale
};
