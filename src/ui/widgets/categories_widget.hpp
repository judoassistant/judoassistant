#pragma once

#include <QWidget>
#include <QTableView>

#include "core/core.hpp"
#include "ui/models/category_matches_model.hpp"
#include "ui/models/results_model.hpp"
#include "ui/delegates/match_card_delegate.hpp"

class CategoriesProxyModel;
class CategoryMatchesWidget;
class CategoryPlayersWidget;
class EditCategoryWidget;
class StoreManager;

class CategoriesWidget : public QWidget {
    Q_OBJECT
public:
    CategoriesWidget(StoreManager & storeManager);
public slots:
    void showCategoryCreateDialog();
    void eraseSelectedCategories();
    void drawSelectedCategories();
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
private:
    void showContextMenu(const QPoint &pos);

    StoreManager &mStoreManager;
    QTableView *mTableView;
    CategoriesProxyModel *mModel;
    EditCategoryWidget *mEditCategoryWidget;
    CategoryPlayersWidget *mCategoryPlayersWidget;
    ResultsModel *mResultsModel;
    CategoryMatchesWidget *mCategoryMatchesWidget;
};

