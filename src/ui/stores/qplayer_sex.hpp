#pragma once

#include <QCoreApplication>
#include <QString>
#include "core/stores/player_sex.hpp"

class QPlayerSex : public PlayerSex {
    Q_DECLARE_TR_FUNCTIONS(QPlayerSex)
public:
    QPlayerSex(const QPlayerSex &other): PlayerSex(other) {}
    QPlayerSex(const PlayerSex &other): PlayerSex(other) {}
    QPlayerSex(int value): PlayerSex(value) {}

    static QPlayerSex fromHumanString(const QString &str); // Parse a string
    QString toHumanString() const; // Returns a string to display in the UI

private:
    std::vector<QString> strings() const; // Returns a vector of all possible names for this sex
};

