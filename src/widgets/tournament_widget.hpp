#pragma once

#include <QMainWindow>
#include <QString>

#include "core.hpp"
#include "stores/qtournament_store.hpp"

class TournamentWidget : public QWidget {
    Q_OBJECT

public:
    TournamentWidget(QTournamentStore &tournament);
};

