#include "widgets/players_widget.hpp"
#include "widgets/models/players_model.hpp"

PlayersWidget::PlayersWidget(QTournamentStore &tournament)
    : mTournament(tournament)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    {
        QToolBar *toolBar = new QToolBar(tr("Players toolbar"), this);
        qDebug() << QIcon::themeName();
        toolBar->addAction(QIcon::fromTheme("document-new"), tr("New player"));
        toolBar->addAction(QIcon::fromTheme("contact-new"), tr("Edit player"));
        toolBar->addAction(QIcon::fromTheme("contact-new"), tr("Delete player"));

        layout->addWidget(toolBar);


    }

    {
        QTableView *tableView = new QTableView(this);
        PlayersModel *model = new PlayersModel(tournament, layout);
        tableView->setModel(model);
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        layout->addWidget(tableView);
    }
    setLayout(layout);
}
