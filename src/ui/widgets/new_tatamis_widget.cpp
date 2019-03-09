#include <QVBoxLayout>
#include <QToolBar>

#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/tatami_widget.hpp"
#include "ui/widgets/new_tatamis_widget.hpp"
#include "ui/widgets/unallocated_blocks_widget.hpp"

NewTatamisWidget::NewTatamisWidget(StoreManager &storeManager)
    : mStoreManager(storeManager)
{
    // QTournamentStore &tournament = mStoreManager.getTournament();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &NewTatamisWidget::beginTournamentReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &NewTatamisWidget::endTournamentReset);

    QVBoxLayout *layout = new QVBoxLayout(this);

    {
        QToolBar *toolBar = new QToolBar(tr("Tatamis toolbar"), this);

        // QAction *createAction = new QAction(QIcon("player-add.svg"), tr("Automaticly add categories to tatamis"));
        // createAction->setStatusTip(tr("Automaticly add categories to tatamis"));
        // toolBar->addAction(createAction);

        // connect(createAction, &QAction::triggered, this, &PlayersWidget::showPlayerCreateDialog);

        // mEraseAction = new QAction(QIcon("player-erase.svg"), tr("Erase the selected players"));
        // mEraseAction->setStatusTip(tr("Erase the selected players"));
        // mEraseAction->setEnabled(false);
        // toolBar->addAction(mEraseAction);
        // connect(mEraseAction, &QAction::triggered, this, &PlayersWidget::eraseSelectedPlayers);

        layout->addWidget(toolBar);
    }

    QWidget *mainSplitWidget = new QWidget(this);
    QHBoxLayout *mainSplit = new QHBoxLayout(mainSplitWidget);

    {
        mainSplit->addWidget(new UnallocatedBlocksWidget(mStoreManager, mainSplitWidget));
    }

    {
        mScene = new QGraphicsScene(this);
        mScene->setItemIndexMethod(QGraphicsScene::NoIndex);

        QGraphicsView *view = new QGraphicsView(this);

        view->setAlignment(Qt::AlignTop|Qt::AlignLeft);
        view->setScene(mScene);
        view->setCacheMode(QGraphicsView::CacheNone);
        view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
        view->setRenderHint(QPainter::Antialiasing, false);
        view->setMinimumSize(300, 300);
        // view->setMaximumWidth(UnallocatedBlockItem::WIDTH + PADDING*2 + 16);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        // mFixedScrollArea = new FixedScrollArea(300, mainSplitWidget);
        mainSplit->addWidget(view);
    }

    mainSplitWidget->setLayout(mainSplit);
    layout->addWidget(mainSplitWidget);

    beginTournamentReset();
    endTournamentReset();

    setLayout(layout);
}

void NewTatamisWidget::beginTournamentReset() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    beginTatamiCountChange();
}

void NewTatamisWidget::endTournamentReset() {
    auto &tournament = mStoreManager.getTournament();

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeAdded, this, &NewTatamisWidget::beginTatamiCountChange));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAdded, this, &NewTatamisWidget::endTatamiCountChange));

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeErased, this, &NewTatamisWidget::beginTatamiCountChange));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisErased, this, &NewTatamisWidget::endTatamiCountChange));

    endTatamiCountChange();
}

void NewTatamisWidget::beginTatamiCountChange() {
    // mFixedScrollArea->clear();
}

void NewTatamisWidget::endTatamiCountChange() {
    // TatamiList & tatamis = mStoreManager.getTournament().getTatamis();

    // for (size_t i = 0; i < tatamis.tatamiCount(); ++i) {
    //     auto *tatami = new TatamiWidget(mStoreManager, {tatamis.getHandle(i)}, mFixedScrollArea);
    //     mFixedScrollArea->addWidget(tatami);
    // }
}

