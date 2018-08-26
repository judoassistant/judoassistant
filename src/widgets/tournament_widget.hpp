#pragma once

#include <QMainWindow>
#include <QString>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

class TournamentWidget : public QWidget {
    Q_OBJECT

public:
    TournamentWidget(QStoreHandler &storeHandler);
};

