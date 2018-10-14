#include <QSplitter>
#include <QToolBar>

#include "widgets/players_widget.hpp"
#include "widgets/create_player_dialog.hpp"

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
        mTableView->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlayersWidget::selectionChanged);
        connect(mTableView, &QTableView::customContextMenuRequested, this, &PlayersWidget::showContextMenu);

        splitter->addWidget(mTableView);
    }

    {
        mEditPlayerWidget = new EditPlayerWidget(storeHandler, splitter);
        splitter->addWidget(mEditPlayerWidget);
    }

    layout->addWidget(splitter);
    setLayout(layout);
}

void PlayersWidget::showPlayerCreateDialog() {
    CreatePlayerDialog dialog(mStoreHandler);

    dialog.exec();
}

void PlayersWidget::showContextMenu(const QPoint &pos) {
    std::vector<PlayerId> playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());
    const TournamentStore &tournament = mStoreHandler.getTournament();

    if (playerIds.empty())
        return;

    std::unordered_set<CategoryId, CategoryId::Hasher> playerCategoryIds; // TODO: Sort this alphabetically
    for (PlayerId playerId : playerIds) {
        const PlayerStore &player = tournament.getPlayer(playerId);
        for (CategoryId categoryId : player.getCategories())
            playerCategoryIds.insert(categoryId);
    }

    QMenu *menu = new QMenu;
    {
        QAction *action = menu->addAction(tr("Create a new player"));
        connect(action, &QAction::triggered, this, &PlayersWidget::showPlayerCreateDialog);
    }
    {
        QAction *action = menu->addAction(tr("Erase selected players"));
        connect(action, &QAction::triggered, this, &PlayersWidget::eraseSelectedPlayers);
    }
    menu->addSeparator();
    {
        QAction *action = menu->addAction(tr("Erase selected players from all categories"));
        action->setEnabled(!playerCategoryIds.empty());
        connect(action, &QAction::triggered, this, &PlayersWidget::eraseSelectedPlayersFromAllCategories);
    }
    {
        QMenu *submenu = menu->addMenu(tr("Erase selected players from category"));
        submenu->setEnabled(!playerCategoryIds.empty());

        for (CategoryId categoryId : playerCategoryIds) {
            const CategoryStore & category = tournament.getCategory(categoryId);
            QAction *action = submenu->addAction(QString::fromStdString(category.getName()));
            connect(action, &QAction::triggered, [&, categoryId](){eraseSelectedPlayersFromCategory(categoryId);});
        }
    }
    {
        QMenu *submenu = menu->addMenu(tr("Add selected players to category"));

        for (const auto & it : tournament.getCategories()) {
            QAction *action = submenu->addAction(QString::fromStdString(it.second->getName()));
            CategoryId categoryId = it.first;
            connect(action, &QAction::triggered, [&, categoryId](){addSelectedPlayersToCategory(categoryId);});
        }
    }

    menu->exec(mTableView->mapToGlobal(pos), 0);
    delete menu;
}

void PlayersWidget::eraseSelectedPlayers() {
    auto playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());
    mStoreHandler.dispatch(std::make_unique<ErasePlayersAction>(mStoreHandler.getTournament(), std::move(playerIds)));
}

void PlayersWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    auto playerIds = mModel->getPlayers(selected);

    mEraseAction->setEnabled(!playerIds.empty());

    if (playerIds.size() == 1)
        mEditPlayerWidget->setPlayer(playerIds.front());
    else
        mEditPlayerWidget->setPlayer(std::nullopt);
}

void PlayersWidget::eraseSelectedPlayersFromAllCategories() {
    std::vector<PlayerId> playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());

    mStoreHandler.dispatch(std::make_unique<ErasePlayersFromAllCategoriesAction>(mStoreHandler.getTournament(), std::move(playerIds)));
}

void PlayersWidget::eraseSelectedPlayersFromCategory(CategoryId categoryId) {
    std::vector<PlayerId> playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());
    mStoreHandler.dispatch(std::make_unique<ErasePlayersFromCategoryAction>(mStoreHandler.getTournament(), categoryId, std::move(playerIds)));
}

void PlayersWidget::addSelectedPlayersToCategory(CategoryId categoryId) {
    std::vector<PlayerId> playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());
    mStoreHandler.dispatch(std::make_unique<AddPlayersToCategoryAction>(mStoreHandler.getTournament(), categoryId, std::move(playerIds)));
}

