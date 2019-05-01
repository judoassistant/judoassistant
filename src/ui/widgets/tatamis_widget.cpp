#include <QApplication>
#include <QToolBar>
#include <QVBoxLayout>

#include "core/log.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/graphics_items/grid_line_graphics_item.hpp"
#include "ui/widgets/graphics_items/empty_concurrent_graphics_item.hpp"
#include "ui/widgets/graphics_items/concurrent_graphics_item.hpp"
#include "ui/widgets/graphics_items/tatami_text_graphics_item.hpp"
#include "ui/widgets/tatamis_widget.hpp"
#include "ui/widgets/unallocated_blocks_widget.hpp"

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
        mScene = new QGraphicsScene(this);
        mScene->setItemIndexMethod(QGraphicsScene::NoIndex);

        mGrid = new GridGraphicsManager(mScene, this);
        mGrid->setMinSize(1300, 1080);

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
        mainSplit->addWidget(view);
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

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisChanged, this, &TatamisWidget::changeTatamis));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesReset, this, &TatamisWidget::endCategoriesReset));

    endTatamiCountChange();
}

void TatamisWidget::beginTatamiCountChange() {
    for (auto & tatami: mTatamis) {
        tatami.clearBlocks();
    }

    mTatamis.clear();
}

void TatamisWidget::endTatamiCountChange() {
    TatamiList & tatamis = mStoreManager.getTournament().getTatamis();

    int minutes = 0;

    for (size_t i = 0; i < tatamis.tatamiCount(); ++i) {
        int x = GridGraphicsManager::HORIZONTAL_OFFSET + i * GridGraphicsManager::GRID_WIDTH;
        int y = GridGraphicsManager::VERTICAL_OFFSET;
        TatamiLocation location{tatamis.getHandle(i)};

        TatamiGraphicsManager tatami(mStoreManager, mScene, location, x, y);
        minutes = std::max(minutes, tatami.getMinutes());
        mTatamis.emplace_back(std::move(tatami));
    }

    mGrid->updateGrid(tatamis.tatamiCount(), minutes, 1900, 1000);
}

GridGraphicsManager::GridGraphicsManager(QGraphicsScene *scene, QWidget *parent)
    : mParent(parent)
    , mScene(scene)
    , mTatamiCount(0)
    , mMinutes(0)
    , mMinWidth(0)
    , mMinHeight(0)
{}

void GridGraphicsManager::setMinutes(int minutes) {
    updateGrid(mTatamiCount, minutes, mMinWidth, mMinHeight);
}

void GridGraphicsManager::setTatamiCount(int tatamiCount) {
    updateGrid(tatamiCount, mMinutes, mMinWidth, mMinHeight);
}

void GridGraphicsManager::setMinSize(int minWidth, int minHeight) {
    updateGrid(mTatamiCount, mMinutes, minWidth, minHeight);
}

void GridGraphicsManager::updateGrid(int tatamiCount, int minutes, int minWidth, int minHeight) {
    for (auto item : mItems) {
        mScene->removeItem(item);
        delete item;
    }

    mItems.clear();

    // TODO: Optimize
    mTatamiCount = tatamiCount;
    mMinutes = minutes;
    mMinWidth = minWidth;
    mMinHeight = minHeight;

    int minuteBound = (mMinutes + GRID_RESOLUTION - 1) / GRID_RESOLUTION;
    int height = std::max(minHeight, VERTICAL_OFFSET + GRID_HEIGHT * minuteBound);
    int width = std::max(mTatamiCount * GRID_WIDTH + HORIZONTAL_OFFSET, mMinWidth);

    QPalette palette = (mParent != nullptr ? mParent->palette() : QApplication::palette());

    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(palette.color(QPalette::ButtonText));

    for (int i = 0; i < mTatamiCount; ++i) {
        auto item = new TatamiTextGraphicsItem(i);
        item->setPos(HORIZONTAL_OFFSET + i * GRID_WIDTH, 0);
        mScene->addItem(item);
        mItems.push_back(item);
    }

    for (int i = VERTICAL_OFFSET, minutes = 0; i <= height; i += GRID_HEIGHT, minutes += GRID_RESOLUTION) {
        auto item = new GridLineGraphicsItem(minutes, width);
        item->setPos(0, i);
        mScene->addItem(item);
        mItems.push_back(item);
    }
}

void TatamisWidget::changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {
    int minutes = 0;
    for (auto & tatami : mTatamis) {
        tatami.changeTatamis(locations, blocks);
        minutes = std::max(minutes, tatami.getMinutes());
    }

    mGrid->setMinutes(minutes);
}

void TatamisWidget::endCategoriesReset() {
    int minutes = 0;
    for (auto & tatami : mTatamis) {
        tatami.reloadBlocks();
        minutes = std::max(minutes, tatami.getMinutes());
    }

    mGrid->setMinutes(minutes);
}

TatamiGraphicsManager::TatamiGraphicsManager(StoreManager & storeManager, QGraphicsScene *scene, TatamiLocation location, int x, int y)
    : mStoreManager(storeManager)
    , mScene(scene)
    , mLocation(location)
    , mX(x)
    , mY(y)
{
    reloadBlocks();
}

int TatamiGraphicsManager::getMinutes() const {
    return mMinutes;
}

void TatamiGraphicsManager::changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {
    std::chrono::milliseconds expectedDuration = std::chrono::milliseconds(0);

    std::unordered_set<PositionId> affectedConcurrentGroups;
    for (auto location : locations) {
        if (location.sequentialGroup.concurrentGroup.tatami != mLocation) continue;

        affectedConcurrentGroups.insert(location.sequentialGroup.concurrentGroup.handle.id);
    }

    if (affectedConcurrentGroups.empty())
        return;

    const auto &tatamis = mStoreManager.getTournament().getTatamis();
    const auto &tatami = tatamis.at(mLocation);

    for (auto i = mGroups.begin(); i != mGroups.end();) {
        auto next = std::next(i);
        PositionHandle handle = (*i)->getLocation().handle;
        if (!tatami.containsGroup(handle)) {
            mScene->removeItem(*i);
            delete (*i);
            mGroups.erase(i);
        }

        i = next;
    }

    // There is always an empty block at the top
    size_t offset = mY + EmptyConcurrentGraphicsItem::HEIGHT;

    auto it = mGroups.begin();
    for (size_t i = 0; i < tatami.groupCount(); ++i) {
        ConcurrentGraphicsItem *item = nullptr;
        const auto &handle = tatami.getHandle(i);
        const auto &group = tatami.at(handle);

        expectedDuration += group.getExpectedDuration();

        if (it == mGroups.end() || handle != ((*it)->getLocation().handle)) {
            // insert group
            ConcurrentGroupLocation location{mLocation, handle};
            item = new ConcurrentGraphicsItem(&mStoreManager, location);
            mScene->addItem(item);
            mGroups.insert(it, item);
        }
        else {
            item = *it;
            if (affectedConcurrentGroups.find(item->getLocation().handle.id) != affectedConcurrentGroups.end())
                item->reloadBlocks();

            std::advance(it, 1);
        }

        item->setPos(mX, offset);
        offset += item->getHeight();

        if (mEmptyGroups.size() < i + 2) {
            mEmptyGroups.push_back(new EmptyConcurrentGraphicsItem(&mStoreManager, mLocation, i+1));
            mScene->addItem(mEmptyGroups.back());
        }

        mEmptyGroups[i+1]->setPos(mX, offset);
        offset += EmptyConcurrentGraphicsItem::HEIGHT;
    }

    while (mEmptyGroups.size() > mGroups.size() + 1) {
        mScene->removeItem(mEmptyGroups.back());
        delete (mEmptyGroups.back());
        mEmptyGroups.pop_back();
    }

    mMinutes = std::chrono::duration_cast<std::chrono::minutes>(expectedDuration).count();
}

void TatamiGraphicsManager::clearBlocks() {
    for (auto &group : mEmptyGroups) {
        mScene->removeItem(group);
        delete group;
    }

    for (auto &group : mGroups) {
        mScene->removeItem(group);
        delete group;
    }
}

void TatamiGraphicsManager::reloadBlocks() {
    std::chrono::milliseconds expectedDuration = std::chrono::milliseconds(0);
    const TournamentStore &tournament = mStoreManager.getTournament();
    const TatamiStore &tatami = tournament.getTatamis().at(mLocation);

    clearBlocks();

    size_t offset = mY;

    {
        auto *emptyItem = new EmptyConcurrentGraphicsItem(&mStoreManager, mLocation, 0);
        mEmptyGroups.push_back(emptyItem);
        mScene->addItem(emptyItem);

        emptyItem->setPos(mX, offset);
        offset += EmptyConcurrentGraphicsItem::HEIGHT;
    }

    for (size_t i = 0; i < tatami.groupCount(); ++i) {
        {
            const auto &handle = tatami.getHandle(i);
            const auto &group = tatami.at(handle);

            expectedDuration += group.getExpectedDuration();

            ConcurrentGroupLocation location{mLocation, handle};
            auto *item = new ConcurrentGraphicsItem(&mStoreManager, location);
            mScene->addItem(item);
            mGroups.push_back(item);

            item->setPos(mX, offset);
            offset += item->getHeight();
        }

        {
            auto *item = new EmptyConcurrentGraphicsItem(&mStoreManager, mLocation, i+1);
            mEmptyGroups.push_back(item);
            mScene->addItem(item);

            item->setPos(mX, offset);
            offset += EmptyConcurrentGraphicsItem::HEIGHT;
        }
    }

    mMinutes = std::chrono::duration_cast<std::chrono::minutes>(expectedDuration).count();
}

