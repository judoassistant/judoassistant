#pragma once

#include <QMainWindow>
#include <QString>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

class TatamisWidget : public QWidget {
    Q_OBJECT

public:
    TatamisWidget(QStoreHandler &storeHandler);
};

