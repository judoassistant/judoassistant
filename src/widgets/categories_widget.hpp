#pragma once

#include <QWidget>
#include <QTableView>

#include "core.hpp"
#include "store_managers/store_manager.hpp"
#include "widgets/models/categories_model.hpp"
#include "widgets/edit_category_widget.hpp"
#include "widgets/edit_category_players_widget.hpp"

class CategoriesWidget : public QWidget {
    Q_OBJECT
public:
    CategoriesWidget(StoreManager & storeManager);
public slots:
    void showCategoryCreateDialog();
    void eraseSelectedCategories();
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
private:
    StoreManager &mStoreManager;
    QAction *mEraseAction;
    QTableView *mTableView;
    CategoriesProxyModel *mModel;
    EditCategoryWidget *mEditCategoryWidget;
    EditCategoryPlayersWidget *mEditCategoryPlayersWidget;
};

