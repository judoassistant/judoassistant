#pragma once

#include <QMainWindow>
#include <QString>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

class MatchesWidget : public QWidget {
    Q_OBJECT

public:
    MatchesWidget(QStoreHandler &storeHandler);
};

