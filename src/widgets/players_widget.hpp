#pragma once

#include <QMainWindow>
#include <QString>
#include <QtWidgets>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

class PlayersWidget : public QWidget {
    Q_OBJECT
public:
    PlayersWidget(QStoreHandler &storeHandler);
public slots:
    void showPlayerCreateDialog();
private:
    QStoreHandler &mStoreHandler;
};

