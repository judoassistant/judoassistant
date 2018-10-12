#pragma once

#include <QMainWindow>
#include <QtWidgets>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"
#include "widgets/models/players_model.hpp"

class PlayersWidget : public QWidget {
    Q_OBJECT
public:
    PlayersWidget(QStoreHandler &storeHandler);
public slots:
    void showPlayerCreateDialog();
    void eraseSelectedPlayers();
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
private:
    QStoreHandler &mStoreHandler;
    QAction *mEraseAction;
    QTableView *mTableView;
    PlayersProxyModel *mModel;
};

