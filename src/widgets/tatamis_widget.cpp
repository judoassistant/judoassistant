#include <QVBoxLayout>
#include <QToolBar>

#include "widgets/tatamis_widget.hpp"
#include "widgets/tatami_widget.hpp"
#include "widgets/unallocated_blocks_widget.hpp"
#include "store_managers/store_manager.hpp"
#include "stores/qtournament_store.hpp"

TatamisWidget::TatamisWidget(StoreManager &storeManager)
    : mStoreManager(storeManager)
{
    // QTournamentStore &tournament = mStoreManager.getTournament();

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &TatamisWidget::beginTournamentReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &TatamisWidget::endTournamentReset);

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
        mFixedScrollArea = new FixedScrollArea(300, mainSplitWidget);
        mainSplit->addWidget(mFixedScrollArea);
    }

    mainSplitWidget->setLayout(mainSplit);
    layout->addWidget(mainSplitWidget);

    beginTournamentReset();
    endTournamentReset();

    setLayout(layout);
}

void TatamisWidget::beginTournamentReset() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    beginTatamiCountChange();
}

void TatamisWidget::endTournamentReset() {
    auto &tournament = mStoreManager.getTournament();

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeAdded, this, &TatamisWidget::beginTatamiCountChange));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAdded, this, &TatamisWidget::endTatamiCountChange));

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeErased, this, &TatamisWidget::beginTatamiCountChange));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisErased, this, &TatamisWidget::endTatamiCountChange));

    endTatamiCountChange();
}

void TatamisWidget::beginTatamiCountChange() {
    mFixedScrollArea->clear();
}

void TatamisWidget::endTatamiCountChange() {
    TatamiList & tatamis = mStoreManager.getTournament().getTatamis();

    for (size_t i = 0; i < tatamis.tatamiCount(); ++i) {
        auto *tatami = new TatamiWidget(mStoreManager, i, mFixedScrollArea);
        mFixedScrollArea->addWidget(tatami);
    }
}

