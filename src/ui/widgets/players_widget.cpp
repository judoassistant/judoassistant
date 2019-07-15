#include <QSplitter>
#include <QToolBar>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMenu>

#include "core/actions/erase_players_action.hpp"
#include "core/actions/erase_players_from_all_categories_action.hpp"
#include "core/actions/erase_players_from_category_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/actions/add_players_to_category_action.hpp"
#include "core/stores/category_store.hpp"
#include "ui/models/players_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/auto_add_category_dialog.hpp"
#include "ui/widgets/create_player_dialog.hpp"
#include "ui/widgets/create_category_dialog.hpp"
#include "ui/widgets/edit_player_widget.hpp"
#include "ui/widgets/players_widget.hpp"

PlayersWidget::PlayersWidget(StoreManager &storeManager)
    : mStoreManager(storeManager)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    {
        QToolBar *toolBar = new QToolBar(tr("Players toolbar"), this);
        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        QAction *createAction = toolBar->addAction(QIcon("icons/player-add.svg"), tr("Create Player"));
        connect(createAction, &QAction::triggered, this, &PlayersWidget::showPlayerCreateDialog);

        QAction *hideAction = toolBar->addAction(QIcon("icons/hide.svg"), tr("Hide Field"));
        connect(hideAction, &QAction::triggered, this, &PlayersWidget::showHideMenu);

        QAction *filterAction = toolBar->addAction(QIcon("icons/filter.svg"), tr("Filter"));
        connect(filterAction, &QAction::triggered, this, &PlayersWidget::showFilterMenu);

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
        QAction *action = menu.addAction(tr("Create new category for selected players..."));
        connect(action, &QAction::triggered, this, &PlayersWidget::showCategoryCreateDialog);
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
    auto playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());

    mEditPlayerWidget->setPlayers(std::move(playerIds));
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

void PlayersWidget::showCategoryCreateDialog() {
    std::vector<PlayerId> playerIds = mModel->getPlayers(mTableView->selectionModel()->selection());

    CreateCategoryDialog dialog(mStoreManager, playerIds, this);

    dialog.exec();
}

void PlayersWidget::showFilterMenu() {

}

void PlayersWidget::showHideMenu() {
    QMenu menu;
    QAction *firstNameAction = menu.addAction(tr("First Name"));
    firstNameAction->setCheckable(true);

    QAction *lastNameAction = menu.addAction(tr("Last Name"));
    lastNameAction->setCheckable(true);

    QAction *sexAction = menu.addAction(tr("Sex"));
    sexAction->setCheckable(true);

    QAction *ageAction = menu.addAction(tr("Age"));
    ageAction->setCheckable(true);

    QAction *weightAction = menu.addAction(tr("Weight"));
    weightAction->setCheckable(true);

    QAction *rankAction = menu.addAction(tr("Rank"));
    rankAction->setCheckable(true);

    QAction *clubAction = menu.addAction(tr("Club"));
    clubAction->setCheckable(true);

    QAction *countryAction = menu.addAction(tr("Country"));
    countryAction->setCheckable(true);

    QAction *categoriesAction = menu.addAction(tr("Categories"));
    categoriesAction->setCheckable(true);

    {
        // QAction *action = menu.addAction(tr("Create a new player"));
        // connect(action, &QAction::triggered, this, &PlayersWidget::showPlayerCreateDialog);
    }
    menu.exec(QCursor::pos());
}

