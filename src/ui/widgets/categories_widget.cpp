#include <QHeaderView>
#include <QListView>
#include <QMenu>
#include <QSplitter>
#include <QTabWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include "core/log.hpp"
#include "core/actions/category_actions.hpp"
#include "core/stores/category_store.hpp"
#include "ui/models/categories_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/categories_widget.hpp"
#include "ui/widgets/create_category_dialog.hpp"
#include "ui/widgets/edit_category_players_widget.hpp"
#include "ui/widgets/edit_category_widget.hpp"

CategoriesWidget::CategoriesWidget(StoreManager & storeManager)
    : mStoreManager(storeManager)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    {
        QToolBar *toolBar = new QToolBar(tr("Categories toolbar"), this);

        QAction *categoryCreateAction = new QAction(QIcon("player-add.svg"), tr("New category"));
        categoryCreateAction->setStatusTip(tr("Create a new category"));
        toolBar->addAction(categoryCreateAction);

        connect(categoryCreateAction, &QAction::triggered, this, &CategoriesWidget::showCategoryCreateDialog);

        mEraseAction = new QAction(QIcon("player-erase.svg"), tr("Erase the selected categories"));
        mEraseAction->setStatusTip(tr("Erase the selected categories"));
        mEraseAction->setEnabled(false);
        toolBar->addAction(mEraseAction);
        connect(mEraseAction, &QAction::triggered, this, &CategoriesWidget::eraseSelectedCategories);

        layout->addWidget(toolBar);
    }

    QSplitter *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->setOrientation(Qt::Horizontal);
    {
        mTableView = new QTableView(this);
        mModel = new CategoriesProxyModel(storeManager, layout);

        mTableView->setModel(mModel);
        mTableView->horizontalHeader()->setStretchLastSection(true);
        mTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSortingEnabled(true);
        mTableView->sortByColumn(0, Qt::AscendingOrder);
        mTableView->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CategoriesWidget::selectionChanged);
        connect(mTableView, &QTableView::customContextMenuRequested, this, &CategoriesWidget::showContextMenu);

        splitter->addWidget(mTableView);
    }

    {
        QTabWidget *tabWidget = new QTabWidget(this);

        mEditCategoryWidget = new EditCategoryWidget(storeManager, splitter);
        tabWidget->addTab(mEditCategoryWidget, tr("General"));

        mEditCategoryPlayersWidget = new EditCategoryPlayersWidget(mStoreManager, this);
        tabWidget->addTab(mEditCategoryPlayersWidget, tr("Players"));

        {
            auto *matchesList = new QListView(this);
            mCategoryMatchesModel = new CategoryMatchesModel(mStoreManager, this);
            matchesList->setItemDelegate(new MatchCardDelegate(this));
            matchesList->setModel(mCategoryMatchesModel);
            tabWidget->addTab(matchesList, tr("Matches"));
        }

        {
            auto *resultsView = new QTableView(this);
            mResultsModel = new ResultsModel(mStoreManager, this);

            resultsView->setModel(mResultsModel);
            resultsView->horizontalHeader()->setStretchLastSection(true);
            resultsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
            resultsView->setSelectionBehavior(QAbstractItemView::SelectRows);
            // resultsView->setSortingEnabled(false);
            // resultsView->sortByColumn(0, Qt::AscendingOrder);

            tabWidget->addTab(resultsView, tr("Results"));
        }

        splitter->addWidget(tabWidget);
    }

    layout->addWidget(splitter);
    setLayout(layout);
}

void CategoriesWidget::showCategoryCreateDialog() {
    CreateCategoryDialog dialog(mStoreManager, this);

    dialog.exec();
}

void CategoriesWidget::eraseSelectedCategories() {
    auto categoryIds = mModel->getCategories(mTableView->selectionModel()->selection());
    mStoreManager.dispatch(std::make_unique<EraseCategoriesAction>(std::move(categoryIds)));
}

void CategoriesWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    auto categoryIds = mModel->getCategories(mTableView->selectionModel()->selection());
    std::optional<CategoryId> categoryId = (categoryIds.size() == 1 ? std::make_optional<CategoryId>(categoryIds.front()) : std::nullopt);

    mEraseAction->setEnabled(!categoryIds.empty());

    mEditCategoryWidget->setCategories(categoryIds);
    mEditCategoryPlayersWidget->setCategory(categoryId);
    mCategoryMatchesModel->setCategory(categoryId);
    mResultsModel->setCategory(categoryId);
}

void CategoriesWidget::showContextMenu(const QPoint &pos) {
    std::vector<CategoryId> categoryIds = mModel->getCategories(mTableView->selectionModel()->selection());
    const TournamentStore &tournament = mStoreManager.getTournament();

    if (categoryIds.empty())
        return;

    QMenu menu;
    {
        QAction *action = menu.addAction(tr("Create a new category"));
        connect(action, &QAction::triggered, this, &CategoriesWidget::showCategoryCreateDialog);
    }
    {
        QAction *action = menu.addAction(tr("Erase selected categories"));
        connect(action, &QAction::triggered, this, &CategoriesWidget::eraseSelectedCategories);
    }
    // menu->addSeparator();
    // {
    //     QAction *action = menu->addAction(tr("Erase selected players from all categories"));
    //     action->setEnabled(!playerCategoryIds.empty());
    //     connect(action, &QAction::triggered, this, &PlayersWidget::eraseSelectedPlayersFromAllCategories);
    // }
    // {
    //     QMenu *submenu = menu->addMenu(tr("Erase selected players from category"));
    //     submenu->setEnabled(!playerCategoryIds.empty());

    //     for (CategoryId categoryId : playerCategoryIds) {
    //         const CategoryStore & category = tournament.getCategory(categoryId);
    //         QAction *action = submenu->addAction(QString::fromStdString(category.getName()));
    //         connect(action, &QAction::triggered, [&, categoryId](){eraseSelectedPlayersFromCategory(categoryId);});
    //     }
    // }
    // {
    //     QMenu *submenu = menu->addMenu(tr("Add selected players to category"));
    //     submenu->setEnabled(!tournament.getCategories().empty());

    //     for (const auto & it : tournament.getCategories()) {
    //         QAction *action = submenu->addAction(QString::fromStdString(it.second->getName()));
    //         CategoryId categoryId = it.first;
    //         connect(action, &QAction::triggered, [&, categoryId](){addSelectedPlayersToCategory(categoryId);});
    //     }
    // }
    // {
    //     QAction *action = menu->addAction(tr("Automatically create categories for the selected players.."));
    //     connect(action, &QAction::triggered, this, &PlayersWidget::showAutoAddCategoriesWidget);
    // }

    menu.exec(mTableView->mapToGlobal(pos), 0);
}
