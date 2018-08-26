#pragma once

#include <QMainWindow>
#include <QString>

#include "core.hpp"
#include "stores/qtournament_store.hpp"

class TatamisWidget : public QWidget {
    Q_OBJECT

public:
    TatamisWidget(QTournamentStore &tournament);
};

