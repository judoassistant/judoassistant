#include <QSplitter>
#include <QToolBar>
#include <QSortFilterProxyModel>

#include "widgets/categories_widget.hpp"
#include "widgets/create_category_dialog.hpp"

CategoriesWidget::CategoriesWidget(QStoreHandler & storeHandler)
    : mStoreHandler(storeHandler)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    {
        QToolBar *toolBar = new QToolBar(tr("Categories toolbar"), this);

        QAction *categoryCreateAction = new QAction(QIcon("player-add.svg"), tr("New category"));
        categoryCreateAction->setStatusTip(tr("Create a new category"));
        toolBar->addAction(categoryCreateAction);

        connect(categoryCreateAction, &QAction::triggered, this, &CategoriesWidget::showCategoryCreateDialog);

        layout->addWidget(toolBar);
    }

    {
        mTableView = new QTableView(this);
        mModel = new CategoriesProxyModel(storeHandler, layout);

        mTableView->setModel(mModel);
        mTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSortingEnabled(true);
        mTableView->sortByColumn(0, Qt::AscendingOrder);

        layout->addWidget(mTableView);
    }
    setLayout(layout);
}

void CategoriesWidget::showCategoryCreateDialog() {
    CreateCategoryDialog dialog(mStoreHandler, this);

    dialog.exec();
}
