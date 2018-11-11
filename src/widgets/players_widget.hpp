#pragma once

#include <QWidget>
#include <QTableView>

#include "core.hpp"
#include "store_managers/store_manager.hpp"
#include "widgets/models/players_model.hpp"
#include "widgets/edit_player_widget.hpp"

class PlayersWidget : public QWidget {
    Q_OBJECT
public:
    PlayersWidget(StoreManager &storeManager);
public slots:
    void showPlayerCreateDialog();
    void showAutoAddCategoriesWidget();
    void eraseSelectedPlayers();
    void eraseSelectedPlayersFromAllCategories();
    void eraseSelectedPlayersFromCategory(CategoryId categoryId);
    void addSelectedPlayersToCategory(CategoryId categoryId);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void showContextMenu(const QPoint &pos);
private:
    StoreManager &mStoreManager;
    QAction *mEraseAction;
    QAction *mAutoAddCategoriesAction;
    QTableView *mTableView;
    PlayersProxyModel *mModel;
    EditPlayerWidget *mEditPlayerWidget;
};

