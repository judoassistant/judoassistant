#include <QSplitter>
#include <QToolBar>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMenu>

#include "core/actions/category_actions.hpp"
#include "core/actions/player_actions.hpp"
#include "core/stores/category_store.hpp"
#include "ui/models/players_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/auto_add_category_dialog.hpp"
#include "ui/widgets/create_player_dialog.hpp"
#include "ui/widgets/edit_player_widget.hpp"
#include "ui/widgets/players_widget.hpp"

PlayersWidget::PlayersWidget(StoreManager &storeManager)
    : mStoreManager(storeManager)
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

        mAutoAddCategoriesAction = new QAction(QIcon("player-erase.svg"), tr("Automatically create categories for the selected players.."));
        mAutoAddCategoriesAction->setStatusTip(tr("Automatically create categories for the selected players.."));
        mAutoAddCategoriesAction->setEnabled(false);
        toolBar->addAction(mAutoAddCategoriesAction);
        connect(mAutoAddCategoriesAction, &QAction::triggered, this, &PlayersWidget::showAutoAddCategoriesWidget);

        layout->addWidget(toolBar);
    }

    QSplitter *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->setOrientation(Qt::Horizontal);

    {
        mTableView = new QTableView(splitter);
        mModel = new PlayersProxyModel(storeManager, layout);

        mTableView->setModel(mModel);
        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->horizontalHeader()->setStretchLastSection(true);
        mTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        mTableView->setSortingEnabled(true);
        mTableView->sortByColumn(1, Qt::AscendingOrder);
        mTableView->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlayersWidget::selectionChanged);
        connect(mTableView, &QTableView::customContextMenuRequested, this, &PlayersWidget::showContextMenu);

        splitter->addWidget(mTableView);
    }

    {
        mEditPlayerWidget = new EditPlayerWidget(storeManager, splitter);
        splitter->addWidget(mEditPlayerWidget);
    }

    layout->addWidget(splitter);
    setLayout(layout);
}

void PlayersWidget::showPlayerCreateDialog() {
    CreatePlayerDialog dialog(mStoreManager);

    dialog.exec();
}

void PlayersWidget::showAutoAddCategoriesWidget() {
    std::vector<PlayerId> playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());
    if (playerIds.empty())
        return;

    bool hasWeights = true;
    for (auto playerId : playerIds) {
        const auto &player = mStoreManager.getTournament().getPlayer(playerId);
        if (!player.getWeight()) {
            hasWeights = false;
            break;
        }
    }

    if (!hasWeights) {
        auto reply = QMessageBox::question(this, tr("Missing weights"), tr("Not all of the selected players have a weight entered. Players with no weight will be ignored. Would you like to continue?"), QMessageBox::Yes | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel)
            return;
    }

    AutoAddCategoryDialog dialog(mStoreManager, playerIds);

    dialog.exec();
}

void PlayersWidget::showContextMenu(const QPoint &pos) {
    std::vector<PlayerId> playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());
    const TournamentStore &tournament = mStoreManager.getTournament();

    if (playerIds.empty())
        return;

    std::unordered_set<CategoryId, CategoryId::Hasher> playerCategoryIds; // TODO: Sort this alphabetically
    for (PlayerId playerId : playerIds) {
        const PlayerStore &player = tournament.getPlayer(playerId);
        for (CategoryId categoryId : player.getCategories())
            playerCategoryIds.insert(categoryId);
    }

    QMenu menu;
    {
        QAction *action = menu.addAction(tr("Create a new player"));
        connect(action, &QAction::triggered, this, &PlayersWidget::showPlayerCreateDialog);
    }
    {
        QAction *action = menu.addAction(tr("Erase selected players"));
        connect(action, &QAction::triggered, this, &PlayersWidget::eraseSelectedPlayers);
    }
    menu.addSeparator();
    {
        QAction *action = menu.addAction(tr("Erase selected players from all categories"));
        action->setEnabled(!playerCategoryIds.empty());
        connect(action, &QAction::triggered, this, &PlayersWidget::eraseSelectedPlayersFromAllCategories);
    }
    {
        QMenu *submenu = menu.addMenu(tr("Erase selected players from category"));
        submenu->setEnabled(!playerCategoryIds.empty());

        for (CategoryId categoryId : playerCategoryIds) {
            const CategoryStore & category = tournament.getCategory(categoryId);
            QAction *action = submenu->addAction(QString::fromStdString(category.getName()));
            connect(action, &QAction::triggered, [&, categoryId](){eraseSelectedPlayersFromCategory(categoryId);});
        }
    }
    {
        QMenu *submenu = menu.addMenu(tr("Add selected players to category"));
        submenu->setEnabled(!tournament.getCategories().empty());

        for (const auto & it : tournament.getCategories()) {
            QAction *action = submenu->addAction(QString::fromStdString(it.second->getName()));
            CategoryId categoryId = it.first;
            connect(action, &QAction::triggered, [&, categoryId](){addSelectedPlayersToCategory(categoryId);});
        }
    }
    {
        QAction *action = menu.addAction(tr("Automatically create categories for the selected players.."));
        connect(action, &QAction::triggered, this, &PlayersWidget::showAutoAddCategoriesWidget);
    }

    menu.exec(mTableView->mapToGlobal(pos), 0);
}

void PlayersWidget::eraseSelectedPlayers() {
    auto playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());
    mStoreManager.dispatch(std::make_unique<ErasePlayersAction>(std::move(playerIds)));
}

void PlayersWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    auto playerIds = mModel->getPlayers(selected);

    mEraseAction->setEnabled(!playerIds.empty());
    mAutoAddCategoriesAction->setEnabled(!playerIds.empty());

    if (playerIds.size() == 1)
        mEditPlayerWidget->setPlayer(playerIds.front());
    else
        mEditPlayerWidget->setPlayer(std::nullopt);
}

void PlayersWidget::eraseSelectedPlayersFromAllCategories() {
    std::vector<PlayerId> playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());

    mStoreManager.dispatch(std::make_unique<ErasePlayersFromAllCategoriesAction>(std::move(playerIds)));
}

void PlayersWidget::eraseSelectedPlayersFromCategory(CategoryId categoryId) {
    std::vector<PlayerId> playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());
    mStoreManager.dispatch(std::make_unique<ErasePlayersFromCategoryAction>(categoryId, std::move(playerIds)));
}

void PlayersWidget::addSelectedPlayersToCategory(CategoryId categoryId) {
    std::vector<PlayerId> playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());
    mStoreManager.dispatch(std::make_unique<AddPlayersToCategoryAction>(categoryId, std::move(playerIds)));
}

