#include "widgets/players_widget.hpp"
#include "widgets/models/players_model.hpp"

PlayersWidget::PlayersWidget(std::unique_ptr<QTournamentStore> &tournament)
    : mTournament(tournament)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QTableView *tableView = new QTableView(this);
    PlayersModel *model = new PlayersModel(tournament, this);
    tableView->setModel(model);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    layout->addWidget(tableView);
    setLayout(layout);
}
