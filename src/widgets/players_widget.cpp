#include "widgets/players_widget.hpp"
#include "widgets/models/players_model.hpp"
#include "widgets/create_player_dialog.hpp"

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

    {
        QTableView *tableView = new QTableView(this);
        PlayersModel *model = new PlayersModel(storeHandler, layout);
        tableView->setModel(model);
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        layout->addWidget(tableView);
    }
    setLayout(layout);
}

void PlayersWidget::showPlayerCreateDialog() {
    CreatePlayerDialog dialog(mStoreHandler, this);

    dialog.exec();
}
