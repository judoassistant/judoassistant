#include <QHeaderView>
#include <QMenu>

#include "core/actions/draw_categories_action.hpp"
#include "core/actions/erase_players_from_category_action.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "ui/models/players_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/widgets/category_players_widget.hpp"

CategoryPlayersWidget::CategoryPlayersWidget (StoreManager & storeManager, QWidget *parent)
    : QTableView(parent)
    , mStoreManager(storeManager)
{
    mModel = new PlayersProxyModel(mStoreManager, this);
    setModel(mModel);
    // TODO: Make this customizeable for the user
    for (int column : {2,3,5,6,7,8})
        setColumnHidden(column, true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setContextMenuPolicy(Qt::CustomContextMenu);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    setSortingEnabled(true);
    sortByColumn(1, Qt::AscendingOrder);

    setCategory(std::nullopt);
    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &CategoryPlayersWidget::tournamentAboutToBeReset);
    connect(this, &QTableView::customContextMenuRequested, this, &CategoryPlayersWidget::showContextMenu);
}

void CategoryPlayersWidget::setCategory(std::optional<CategoryId> categoryId) {
    if (categoryId) {
        mModel->setCategory(categoryId);
        setEnabled(true);
    }
    else {
        mModel->hideAll();
        setEnabled(false);
    }
}

void CategoryPlayersWidget::tournamentAboutToBeReset() {
    setCategory(std::nullopt);
}

void CategoryPlayersWidget::showContextMenu(const QPoint &pos) {
    std::vector<PlayerId> playerIds = mModel->getPlayers(selectionModel()->selection());

    QMenu menu;
    {
        QAction *action = menu.addAction(tr("Erase selected players from category"));
        connect(action, &QAction::triggered, this, &CategoryPlayersWidget::eraseSelectedPlayersFromCategory);
    }

    menu.exec(QCursor::pos() - QPoint(4, 4));
}

void CategoryPlayersWidget::eraseSelectedPlayersFromCategory() {
    std::vector<PlayerId> playerIds = mModel->getPlayers(selectionModel()->selection());

    if (playerIds.empty())
        return;

    if (!mModel->getCategory().has_value())
        return;
    CategoryId categoryId = mModel->getCategory().value();

    mStoreManager.dispatch(std::make_unique<ErasePlayersFromCategoryAction>(categoryId, std::move(playerIds)));
}

