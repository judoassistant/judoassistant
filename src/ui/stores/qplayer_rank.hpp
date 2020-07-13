#pragma once

#include <QCoreApplication>
#include <QString>
#include "core/stores/player_rank.hpp"

class QPlayerRank : public PlayerRank {
    Q_DECLARE_TR_FUNCTIONS(QPlayerRank)
public:
    QPlayerRank(const QPlayerRank &other): PlayerRank(other) {}
    QPlayerRank(const PlayerRank &other): PlayerRank(other) {}
    QPlayerRank(int value): PlayerRank(value) {}

    static QPlayerRank fromHumanString(const QString &str); // Parse a string
    QString toHumanString() const; // Returns a string to display in the UI

private:
    std::vector<QString> strings() const; // Returns a vector of all possible names for this rank
};

