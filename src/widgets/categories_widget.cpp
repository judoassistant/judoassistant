#include "widgets/categories_widget.hpp"
#include "widgets/create_category_dialog.hpp"
#include "widgets/models/categories_model.hpp"

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
        QTableView *tableView = new QTableView(this);
        CategoriesModel *model = new CategoriesModel(storeHandler, layout);
        tableView->setModel(model);
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        layout->addWidget(tableView);
    }
    setLayout(layout);
}

void CategoriesWidget::showCategoryCreateDialog() {
    CreateCategoryDialog dialog(mStoreHandler, this);

    dialog.exec();
}
