#include <QSplitter>
#include <QToolBar>
#include <QSortFilterProxyModel>

#include "widgets/players_widget.hpp"
#include "widgets/models/players_model.hpp"
#include "widgets/create_player_dialog.hpp"
#include "widgets/edit_player_widget.hpp"

PlayersWidget::PlayersWidget(QStoreHandler &storeHandler)
    : mStoreHandler(storeHandler)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    {
        QToolBar *toolBar = new QToolBar(tr("Players toolbar"), this);

        QAction *playerCreateAction = new QAction(QIcon("player-add.svg"), tr("New player"));
        playerCreateAction->setStatusTip(tr("Create a new player"));
        toolBar->addAction(playerCreateAction);

        connect(playerCreateAction, &QAction::triggered, this, &PlayersWidget::showPlayerCreateDialog);

        toolBar->addAction(QIcon("player-delete.svg"), tr("Edit player"));
        toolBar->addAction(QIcon("player-delete.svg"), tr("Delete player"));

        layout->addWidget(toolBar);


    }

    QSplitter *splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);

    {
        QTableView *tableView = new QTableView(splitter);

        PlayersModel *model = new PlayersModel(storeHandler, layout);
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(layout);
        proxyModel->setSourceModel(model);

        tableView->setModel(proxyModel);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tableView->setSortingEnabled(true);
        tableView->sortByColumn(1, Qt::AscendingOrder);

        splitter->addWidget(tableView);
    }

    {
        EditPlayerWidget *editWidget = new EditPlayerWidget(storeHandler, splitter);
        splitter->addWidget(editWidget);
    }

    layout->addWidget(splitter);
    setLayout(layout);
}

void PlayersWidget::showPlayerCreateDialog() {
    CreatePlayerDialog dialog(mStoreHandler, this);

    dialog.exec();
}
