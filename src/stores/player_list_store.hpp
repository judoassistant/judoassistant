#pragma once

#include <QObject>
#include <vector>
#include "stores/player_store.hpp"

class PlayerListStore : public QObject {
    Q_OBJECT
public:
    PlayerListStore();
public slots:
    // void setValue(int value);
signals:
    // void valueChanged(int value);
private:
    std::vector<PlayerStore> m_players;
};

