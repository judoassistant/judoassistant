#include <QSplitter>
#include <QToolBar>

#include "widgets/players_widget.hpp"
#include "widgets/create_player_dialog.hpp"
#include "widgets/edit_player_widget.hpp"

#include "actions/player_actions.hpp"

PlayersWidget::PlayersWidget(QStoreHandler &storeHandler)
    : mStoreHandler(storeHandler)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    {
        QToolBar *toolBar = new QToolBar(tr("Players toolbar"), this);

        QAction *createAction = new QAction(QIcon("player-add.svg"), tr("Create a new player"));
        createAction->setStatusTip(tr("Create a new player"));
        toolBar->addAction(createAction);

        connect(createAction, &QAction::triggered, this, &PlayersWidget::showPlayerCreateDialog);

        mEraseAction = new QAction(QIcon("player-erase.svg"), tr("Erase the selected players"));
        mEraseAction->setStatusTip(tr("Erase the selected players"));
        mEraseAction->setEnabled(false);
        toolBar->addAction(mEraseAction);
        connect(mEraseAction, &QAction::triggered, this, &PlayersWidget::eraseSelectedPlayers);

        layout->addWidget(toolBar);
    }

    QSplitter *splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);

    {
        mTableView = new QTableView(splitter);
        mModel = new PlayersProxyModel(storeHandler, layout);

        mTableView->setModel(mModel);
        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        mTableView->setSortingEnabled(true);
        mTableView->sortByColumn(1, Qt::AscendingOrder);

        connect(mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlayersWidget::selectionChanged);

        splitter->addWidget(mTableView);
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

void PlayersWidget::eraseSelectedPlayers() {
    auto playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());
    mStoreHandler.dispatch(std::make_unique<ErasePlayersAction>(mStoreHandler.getTournament(), std::move(playerIds)));
}

void PlayersWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    mEraseAction->setEnabled(selected.indexes().size() > 0);
}
