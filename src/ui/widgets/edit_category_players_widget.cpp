#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>

#include "ui/models/players_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/widgets/edit_category_players_widget.hpp"

EditCategoryPlayersWidget::EditCategoryPlayersWidget (StoreManager & storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    QVBoxLayout *layout = new QVBoxLayout;
    mTableView = new QTableView(this);
    mModel = new PlayersProxyModel(mStoreManager, this);
    mTableView->setModel(mModel);
    // TODO: Make this customizeable for the user
    for (int column : {2,3,5,6,7,8})
        mTableView->setColumnHidden(column, true);
    mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableView->horizontalHeader()->setStretchLastSection(true);
    mTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    mTableView->setSortingEnabled(true);
    mTableView->sortByColumn(1, Qt::AscendingOrder);

    layout->addWidget(mTableView);


    QHBoxLayout *controlsLayout = new QHBoxLayout(this);

    QPushButton *removeButton = new QPushButton("Remove player from category", this);
    controlsLayout->addWidget(removeButton);

    QWidget *buttonsWidget = new QWidget(this);
    buttonsWidget->setLayout(controlsLayout);

    layout->addWidget(buttonsWidget);

    setLayout(layout);

    setCategory(std::nullopt);
    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &EditCategoryPlayersWidget::tournamentAboutToBeReset);
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

void EditCategoryPlayersWidget::tournamentAboutToBeReset() {
    setCategory(std::nullopt);
}

