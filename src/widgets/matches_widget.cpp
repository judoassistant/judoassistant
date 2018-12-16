#include <QScrollArea>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListView>

#include "widgets/matches_widget.hpp"
#include "widgets/delegates/match_card_delegate.hpp"
#include "widgets/models/tatami_matches_model.hpp"
#include "widgets/models/match_card.hpp"
#include "store_managers/store_manager.hpp"
#include "stores/qtournament_store.hpp"

MatchesWidget::MatchesWidget(StoreManager & storeManager)
    : mStoreManager(storeManager)
{
    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &MatchesWidget::beginTournamentReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &MatchesWidget::endTournamentReset);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mFixedScrollArea = new FixedScrollArea(300, this);
    mainLayout->addWidget(mFixedScrollArea);

    setLayout(mainLayout);

    beginTournamentReset();
    endTournamentReset();
}

void MatchesWidget::beginTournamentReset() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    beginTatamiCountChange();
}

void MatchesWidget::endTournamentReset() {
    auto &tournament = mStoreManager.getTournament();

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeAdded, this, &MatchesWidget::beginTatamiCountChange));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAdded, this, &MatchesWidget::endTatamiCountChange));

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeErased, this, &MatchesWidget::beginTatamiCountChange));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisErased, this, &MatchesWidget::endTatamiCountChange));

    endTatamiCountChange();
}

void MatchesWidget::beginTatamiCountChange() {
    mFixedScrollArea->clear();
}

void MatchesWidget::endTatamiCountChange() {
    // TODO: Synchronize scroll bars
    TatamiList & tatamis = mStoreManager.getTournament().getTatamis();
    for (size_t i = 0; i < tatamis.tatamiCount(); ++i) {
        auto *tatami = new QListView(mFixedScrollArea);
        tatami->setItemDelegate(new MatchCardDelegate(tatami));
        tatami->setModel(new TatamiMatchesModel(mStoreManager, i, 20, this));
        mFixedScrollArea->addWidget(tatami);
    }
}
