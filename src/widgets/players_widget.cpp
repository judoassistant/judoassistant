#include "widgets/players_widget.hpp"
#include "widgets/models/players_model.hpp"

PlayersWidget::PlayersWidget(QStoreHandler &storeHandler)
    : mStoreHandler(storeHandler)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    {
        QToolBar *toolBar = new QToolBar(tr("Players toolbar"), this);
        qDebug() << QIcon::themeName();
        toolBar->addAction(QIcon("player-add.svg"), tr("New player"));
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
