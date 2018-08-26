#pragma once

#include <QMainWindow>
#include <QString>

#include "core.hpp"
#include "stores/qtournament_store.hpp"

class CategoriesWidget : public QWidget {
    Q_OBJECT

public:
    CategoriesWidget(std::unique_ptr<QTournamentStore> &tournament);
};

