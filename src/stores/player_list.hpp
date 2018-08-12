#pragma once

#include <QObject>
#include <vector>
#include "stores/player.hpp"

class PlayerList : public QObject {
    Q_OBJECT
public:
    PlayerList();
public slots:
    // void setValue(int value);
signals:
    // void valueChanged(int value);
private:
    std::vector<Player> m_players;
};

