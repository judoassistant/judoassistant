#include <QHBoxLayout>
#include <QGraphicsView>

#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/new_matches_widget.hpp"
#include "ui/widgets/graphics_items/tatami_text_graphics_item.hpp"

MatchesGridGraphicsManager::MatchesGridGraphicsManager(QGraphicsScene *scene)
    : mScene(scene)
{}

void MatchesGridGraphicsManager::updateGrid(unsigned int tatamiCount) {
    for (auto item : mItems) {
        mScene->removeItem(item);
        delete item;
    }

    mItems.clear();

    for (unsigned int i = 0; i < tatamiCount; ++i) {
        auto item = new TatamiTextGraphicsItem(i);
        item->setPos(HORIZONTAL_OFFSET + i * GRID_WIDTH, 0);
        mScene->addItem(item);
        mItems.push_back(item);
    }
}

NewMatchesWidget::NewMatchesWidget(StoreManager &storeManager)
    : mStoreManager(storeManager)
{
    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &NewMatchesWidget::beginTournamentReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &NewMatchesWidget::endTournamentReset);

    mScene = new QGraphicsScene(this);
    mScene->setItemIndexMethod(QGraphicsScene::NoIndex);

    mGrid = new MatchesGridGraphicsManager(mScene);

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
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    // mFixedScrollArea = new FixedScrollArea(300, mainSplitWidget);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(view);
}

void NewMatchesWidget::beginTournamentReset() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    beginTatamiCountChange();
}

void NewMatchesWidget::endTournamentReset() {
    auto &tournament = mStoreManager.getTournament();

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeAdded, this, &NewMatchesWidget::beginTatamiCountChange));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAdded, this, &NewMatchesWidget::endTatamiCountChange));

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeErased, this, &NewMatchesWidget::beginTatamiCountChange));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisErased, this, &NewMatchesWidget::endTatamiCountChange));

    endTatamiCountChange();
}

void NewMatchesWidget::beginTatamiCountChange() {
    mTatamis.clear();
}

void NewMatchesWidget::endTatamiCountChange() {
    TatamiList &tatamis = mStoreManager.getTournament().getTatamis();

    for (size_t i = 0; i < tatamis.tatamiCount(); ++i) {
        int x = MatchesGridGraphicsManager::HORIZONTAL_OFFSET + i * MatchesGridGraphicsManager::GRID_WIDTH;
        int y = MatchesGridGraphicsManager::VERTICAL_OFFSET;
        TatamiLocation location{tatamis.getHandle(i)};

        MatchesGraphicsManager tatami(mStoreManager, mScene, location, x, y);
        mTatamis.push_back(std::move(tatami));
    }

    mGrid->updateGrid(tatamis.tatamiCount());
}

MatchesGraphicsManager::MatchesGraphicsManager(StoreManager &storeManager, QGraphicsScene *scene, TatamiLocation location, int x, int y) {

}
