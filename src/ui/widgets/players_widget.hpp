#pragma once

#include <QWidget>
#include <QTableView>

#include "core/core.hpp"
#include "core/id.hpp"

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
    void showCategoryCreateDialog();
    void eraseSelectedPlayers();
    void eraseSelectedPlayersFromAllCategories();
    void eraseSelectedPlayersFromCategory(CategoryId categoryId);
    void addSelectedPlayersToCategory(CategoryId categoryId);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void showContextMenu(const QPoint &pos);
    void showFilterMenu();
    void showHideMenu();
private:
    StoreManager &mStoreManager;
    QTableView *mTableView;
    PlayersProxyModel *mModel;
    EditPlayerWidget *mEditPlayerWidget;
};

