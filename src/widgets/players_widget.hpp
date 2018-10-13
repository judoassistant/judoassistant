#pragma once

#include <QMainWindow>
#include <QtWidgets>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"
#include "widgets/models/players_model.hpp"
#include "widgets/edit_player_widget.hpp"

class PlayersWidget : public QWidget {
    Q_OBJECT
public:
    PlayersWidget(QStoreHandler &storeHandler);
public slots:
    void showPlayerCreateDialog();
    void eraseSelectedPlayers();
    void eraseSelectedPlayersFromAllCategories();
    void eraseSelectedPlayersFromCategory(CategoryId categoryId);
    void addSelectedPlayersToCategory(CategoryId categoryId);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void showContextMenu(const QPoint &pos);
private:
    QStoreHandler &mStoreHandler;
    QAction *mEraseAction;
    QTableView *mTableView;
    PlayersProxyModel *mModel;
    EditPlayerWidget *mEditPlayerWidget;
};

