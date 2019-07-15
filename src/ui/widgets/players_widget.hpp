#pragma once

#include <QWidget>
#include <QTableView>

#include "core/core.hpp"
#include "core/id.hpp"

class MasterStoreManager;
class EditPlayerWidget;
class PlayersProxyModel;

class PlayersWidget : public QWidget {
    Q_OBJECT
public:
    PlayersWidget(MasterStoreManager &storeManager);

protected:
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
    void toggleColumn(const QString &key, int column, bool checked);
    void updateHideActionText();
    void updateFilterActionText();

private:
    MasterStoreManager &mStoreManager;
    QTableView *mTableView;
    PlayersProxyModel *mModel;
    EditPlayerWidget *mEditPlayerWidget;
    QAction *mHideAction;
    QAction *mFilterAction;
};

