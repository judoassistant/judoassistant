#pragma once

#include <QWidget>
#include <QTableView>

#include "core.hpp"
#include "id.hpp"

class StoreManager;
class EditPlayerWidget;
class PlayersProxyModel;

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

