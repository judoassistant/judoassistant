#pragma once

#include <QMainWindow>
#include <QString>

#include "core.hpp"
#include "stores/qtournament_store.hpp"

class MatchesWidget : public QWidget {
    Q_OBJECT

public:
    MatchesWidget(std::unique_ptr<QTournamentStore> &tournament);
};

