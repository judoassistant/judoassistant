#include "widgets/edit_category_players_widget.hpp"
#include <QVBoxLayout>
#include <QHeaderView>

EditCategoryPlayersWidget::EditCategoryPlayersWidget (QStoreHandler & storeHandler, QWidget *parent)
    : QWidget(parent)
    , mStoreHandler(storeHandler)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    mTableView = new QTableView(this);
    mModel = new PlayersProxyModel(mStoreHandler, this);
    mTableView->setModel(mModel);
    // TODO: Make this customizeable for the user
    for (int column : {2,3,4,6,7,8})
        mTableView->setColumnHidden(column, true);
    mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableView->horizontalHeader()->setStretchLastSection(true);
    mTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    mTableView->setSortingEnabled(true);
    mTableView->sortByColumn(1, Qt::AscendingOrder);

    layout->addWidget(mTableView);
    setLayout(layout);

    setCategory(std::nullopt);
    connect(&mStoreHandler, &QStoreHandler::tournamentReset, this, &EditCategoryPlayersWidget::tournamentReset);
}

void EditCategoryPlayersWidget::setCategory(std::optional<CategoryId> categoryId) {
    if (categoryId) {
        mModel->setCategory(categoryId);
        mTableView->setEnabled(true);
    }
    else {
        mModel->hideAll();
        mTableView->setEnabled(false);
    }
}

void EditCategoryPlayersWidget::tournamentReset() {
    setCategory(std::nullopt);
}

