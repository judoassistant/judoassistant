#pragma once

#include <QWidget>
#include <QTableView>

#include "core.hpp"
#include "models/category_matches_model.hpp"
#include "delegates/match_card_delegate.hpp"

class StoreManager;
class EditCategoryWidget;
class EditCategoryPlayersWidget;
class CategoriesProxyModel;

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
    CategoryMatchesModel *mCategoryMatchesModel;
};

