#include <QSplitter>
#include <QToolBar>
#include <QSortFilterProxyModel>

#include "widgets/categories_widget.hpp"
#include "widgets/create_category_dialog.hpp"

#include "actions/category_actions.hpp"

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

        mEraseAction = new QAction(QIcon("player-erase.svg"), tr("Erase the selected categories"));
        mEraseAction->setStatusTip(tr("Erase the selected categories"));
        mEraseAction->setEnabled(false);
        toolBar->addAction(mEraseAction);
        connect(mEraseAction, &QAction::triggered, this, &CategoriesWidget::eraseSelectedCategories);

        layout->addWidget(toolBar);
    }

    {
        mTableView = new QTableView(this);
        mModel = new CategoriesProxyModel(storeHandler, layout);

        mTableView->setModel(mModel);
        mTableView->horizontalHeader()->setStretchLastSection(true);
        mTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSortingEnabled(true);
        mTableView->sortByColumn(0, Qt::AscendingOrder);

        connect(mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CategoriesWidget::selectionChanged);

        layout->addWidget(mTableView);
    }
    setLayout(layout);
}

void CategoriesWidget::showCategoryCreateDialog() {
    CreateCategoryDialog dialog(mStoreHandler, this);

    dialog.exec();
}

void CategoriesWidget::eraseSelectedCategories() {
    auto categoryIds = mModel->getCategories(mTableView->selectionModel()->selection());
    mStoreHandler.dispatch(std::make_unique<EraseCategoriesAction>(mStoreHandler.getTournament(), std::move(categoryIds)));
}

void CategoriesWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    mEraseAction->setEnabled(selected.indexes().size() > 0);
}
