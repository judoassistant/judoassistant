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

    // TODO: Handle erase and adding of tatamis
    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &TatamisWidget::tournamentAboutToBeReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &TatamisWidget::tournamentReset);

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
        QScrollArea *scroll = new QScrollArea(mainSplitWidget);
        mTatamiLayout = new QHBoxLayout(scroll);
        TatamiList & tatamis = mStoreManager.getTournament().getTatamis();
        for (size_t i = 0; i < tatamis.tatamiCount(); ++i) {
            auto *tatami = new TatamiWidget(mStoreManager, i, this);
            mTatamis.push_back(tatami);
            mTatamiLayout->addWidget(tatami);
        }
        QWidget *widget = new QWidget(scroll);
        widget->setLayout(mTatamiLayout);
        scroll->setWidget(widget);
        mainSplit->addWidget(scroll);
    }

    mainSplitWidget->setLayout(mainSplit);
    layout->addWidget(mainSplitWidget);

    setLayout(layout);
}

void TatamisWidget::tournamentAboutToBeReset() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    tatamiCountAboutToBeChanged();
}

void TatamisWidget::tournamentReset() {
    auto &tournament = mStoreManager.getTournament();

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeAdded, this, &TatamisWidget::tatamiCountAboutToBeChanged));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAdded, this, &TatamisWidget::tatamiCountChanged));

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeErased, this, &TatamisWidget::tatamiCountAboutToBeChanged));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisErased, this, &TatamisWidget::tatamiCountChanged));

    tatamiCountChanged();
}

void TatamisWidget::tatamiCountAboutToBeChanged() {
    for (TatamiWidget *tatami : mTatamis) {
        mTatamiLayout->removeWidget(tatami);
        delete tatami;
    }

    mTatamis.clear();
}

void TatamisWidget::tatamiCountChanged() {
    TatamiList & tatamis = mStoreManager.getTournament().getTatamis();
    for (size_t i = 0; i < tatamis.tatamiCount(); ++i) {
        auto *tatami = new TatamiWidget(mStoreManager, i, this);
        mTatamis.push_back(tatami);
        mTatamiLayout->addWidget(tatami);
    }
}

