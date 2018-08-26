#pragma once

#include <QMainWindow>
#include <QString>
#include <QtWidgets>

#include "core.hpp"
#include "stores/qtournament_store.hpp"

class PlayersWidget : public QWidget {
    Q_OBJECT

public:
    PlayersWidget(std::unique_ptr<QTournamentStore> &tournament);
private:
    std::unique_ptr<QTournamentStore> &mTournament;
};

