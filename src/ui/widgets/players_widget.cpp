#include <QSplitter>
#include <QToolBar>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMenu>
#include <QSettings>

#include "core/actions/add_players_to_category_action.hpp"
#include "core/actions/draw_categories_action.hpp"
#include "core/actions/erase_players_action.hpp"
#include "core/actions/erase_players_from_all_categories_action.hpp"
#include "core/actions/erase_players_from_category_action.hpp"
#include "core/stores/category_store.hpp"
#include "ui/misc/category_name_comparator.hpp"
#include "ui/models/players_model.hpp"
#include "ui/store_managers/master_store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/auto_add_category_dialog.hpp"
#include "ui/widgets/create_category_dialog.hpp"
#include "ui/widgets/create_player_dialog.hpp"
#include "ui/widgets/edit_player_widget.hpp"
#include "ui/widgets/players_widget.hpp"

PlayersWidget::PlayersWidget(MasterStoreManager &storeManager)
    : mStoreManager(storeManager)
{
    const QSettings& settings = mStoreManager.getSettings();

    QVBoxLayout *layout = new QVBoxLayout(this);

    {
        QToolBar *toolBar = new QToolBar(tr("Players toolbar"), this);
        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        QAction *createAction = toolBar->addAction(QIcon("icons/player-add.svg"), tr("Create Player"));
        connect(createAction, &QAction::triggered, this, &PlayersWidget::showPlayerCreateDialog);

        mHideAction = toolBar->addAction(QIcon("icons/hide.svg"), tr("Hide Fields"));
        connect(mHideAction, &QAction::triggered, this, &PlayersWidget::showHideMenu);
        updateHideActionText();

        // QAction *filterAction = toolBar->addAction(QIcon("icons/filter.svg"), tr("Filter"));
        // connect(filterAction, &QAction::triggered, this, &PlayersWidget::showFilterMenu);

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

        mTableView->setColumnHidden(0, !settings.value("players/showFirstName", true).toBool());
        mTableView->setColumnHidden(1, !settings.value("players/showLastName", true).toBool());
        mTableView->setColumnHidden(2, !settings.value("players/showSex", true).toBool());
        mTableView->setColumnHidden(3, !settings.value("players/showAge", true).toBool());
        mTableView->setColumnHidden(4, !settings.value("players/showWeight", true).toBool());
        mTableView->setColumnHidden(5, !settings.value("players/showRank", true).toBool());
        mTableView->setColumnHidden(6, !settings.value("players/showClub", true).toBool());
        mTableView->setColumnHidden(7, !settings.value("players/showCountry", true).toBool());
        mTableView->setColumnHidden(8, !settings.value("players/showCategories", true).toBool());

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

    std::unordered_set<CategoryId, CategoryId::Hasher> playerCategoryIds;
    for (PlayerId playerId : playerIds) {
        const PlayerStore &player = tournament.getPlayer(playerId);
        for (CategoryId categoryId : player.getCategories())
            playerCategoryIds.insert(categoryId);
    }

    // sort ids by name
    std::vector sortedPlayerCategoryIds(playerCategoryIds.begin(), playerCategoryIds.end());
    std::sort(sortedPlayerCategoryIds.begin(), sortedPlayerCategoryIds.end(), CategoryNameComparator(tournament));

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

        for (CategoryId categoryId : sortedPlayerCategoryIds) {
            const CategoryStore & category = tournament.getCategory(categoryId);
            QAction *action = submenu->addAction(QString::fromStdString(category.getName()));
            connect(action, &QAction::triggered, [&, categoryId](){eraseSelectedPlayersFromCategory(categoryId);});
        }
    }
    {
        QMenu *submenu = menu.addMenu(tr("Add selected players to category"));
        submenu->setEnabled(!tournament.getCategories().empty());

        std::vector<CategoryId> sortedCategoryIds;
        for (const auto & it : tournament.getCategories())
            sortedCategoryIds.push_back(it.first);

        std::sort(sortedCategoryIds.begin(), sortedCategoryIds.end(), CategoryNameComparator(tournament));

        for (const CategoryId &categoryId : sortedCategoryIds) {
            const CategoryStore &category = tournament.getCategory(categoryId);
            QAction *action = submenu->addAction(QString::fromStdString(category.getName()));
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

    menu.exec(QCursor::pos() - QPoint(4, 4));
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
    const QSettings& settings = mStoreManager.getSettings();

    QMenu menu;
    QAction *firstNameAction = menu.addAction(tr("First Name"));
    firstNameAction->setCheckable(true);
    firstNameAction->setChecked(settings.value("players/showFirstName", true).toBool());
    connect(firstNameAction, &QAction::toggled, [this](bool checked) { toggleColumn("players/showFirstName", 0, checked); });

    QAction *lastNameAction = menu.addAction(tr("Last Name"));
    lastNameAction->setCheckable(true);
    lastNameAction->setChecked(settings.value("players/showLastName", true).toBool());
    connect(lastNameAction, &QAction::toggled, [this](bool checked) { toggleColumn("players/showLastName", 1, checked); });

    QAction *sexAction = menu.addAction(tr("Sex"));
    sexAction->setCheckable(true);
    sexAction->setChecked(settings.value("players/showSex", true).toBool());
    connect(sexAction, &QAction::toggled, [this](bool checked) { toggleColumn("players/showSex", 2, checked); });

    QAction *ageAction = menu.addAction(tr("Age"));
    ageAction->setCheckable(true);
    ageAction->setChecked(settings.value("players/showAge", true).toBool());
    connect(ageAction, &QAction::toggled, [this](bool checked) { toggleColumn("players/showAge", 3, checked); });

    QAction *weightAction = menu.addAction(tr("Weight"));
    weightAction->setCheckable(true);
    weightAction->setChecked(settings.value("players/showWeight", true).toBool());
    connect(weightAction, &QAction::toggled, [this](bool checked) { toggleColumn("players/showWeight", 4, checked); });

    QAction *rankAction = menu.addAction(tr("Rank"));
    rankAction->setCheckable(true);
    rankAction->setChecked(settings.value("players/showRank", true).toBool());
    connect(rankAction, &QAction::toggled, [this](bool checked) { toggleColumn("players/showRank", 5, checked); });

    QAction *clubAction = menu.addAction(tr("Club"));
    clubAction->setCheckable(true);
    clubAction->setChecked(settings.value("players/showClub", true).toBool());
    connect(clubAction, &QAction::toggled, [this](bool checked) { toggleColumn("players/showClub", 6, checked); });

    QAction *countryAction = menu.addAction(tr("Country"));
    countryAction->setCheckable(true);
    countryAction->setChecked(settings.value("players/showCountry", true).toBool());
    connect(countryAction, &QAction::toggled, [this](bool checked) { toggleColumn("players/showCountry", 7, checked); });

    QAction *categoriesAction = menu.addAction(tr("Categories"));
    categoriesAction->setCheckable(true);
    categoriesAction->setChecked(settings.value("players/showCategories", true).toBool());
    connect(categoriesAction, &QAction::toggled, [this](bool checked) { toggleColumn("players/showCategories", 8, checked); });

    {
        // QAction *action = menu.addAction(tr("Create a new player"));
        // connect(action, &QAction::triggered, this, &PlayersWidget::showPlayerCreateDialog);
    }
    menu.exec(QCursor::pos() - QPoint(4, 4));
}

void PlayersWidget::toggleColumn(const QString &key, int column, bool checked) {
    QSettings& settings = mStoreManager.getSettings();
    mTableView->setColumnHidden(column, !checked);
    settings.setValue(key, checked);
    updateHideActionText();
}

void PlayersWidget::updateHideActionText() {
    const QSettings& settings = mStoreManager.getSettings();
    unsigned int count = 0;
    count += !settings.value("players/showFirstName", true).toBool();
    count += !settings.value("players/showLastName", true).toBool();
    count += !settings.value("players/showSex", true).toBool();
    count += !settings.value("players/showAge", true).toBool();
    count += !settings.value("players/showWeight", true).toBool();
    count += !settings.value("players/showRank", true).toBool();
    count += !settings.value("players/showClub", true).toBool();
    count += !settings.value("players/showCountry", true).toBool();
    count += !settings.value("players/showCategories", true).toBool();

    if (count == 0)
        mHideAction->setText(tr("Hide Fields"));
    else
        mHideAction->setText(tr("%1 Hidden Field(s)", "", count).arg(count));
}

