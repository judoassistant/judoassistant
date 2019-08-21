#include <QHBoxLayout>
#include <QGraphicsView>

#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/graphics_items/tatami_text_graphics_item.hpp"
#include "ui/widgets/new_matches_widget.hpp"

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

        auto tatami = std::make_unique<MatchesGraphicsManager>(mStoreManager, mScene, location, x, y);
        mTatamis.push_back(std::move(tatami));
    }

    mGrid->updateGrid(tatamis.tatamiCount());
}

MatchesGraphicsManager::MatchesGraphicsManager(StoreManager &storeManager, QGraphicsScene *scene, TatamiLocation location, int x, int y)
    : mStoreManager(storeManager)
    , mLocation(location)
    , mX(x)
    , mY(y)
{
    auto &tournament = mStoreManager.getTournament();
    connect(&mTimer, &QTimer::timeout, this, &MatchesGraphicsManager::timerHit);
    connect(&tournament, &QTournamentStore::matchesChanged, this, &MatchesGraphicsManager::changeMatches);
    connect(&tournament, &QTournamentStore::tatamisChanged, this, &MatchesGraphicsManager::changeTatamis);
    connect(&tournament, &QTournamentStore::playersChanged, this, &MatchesGraphicsManager::changePlayers);
    connect(&tournament, &QTournamentStore::matchesAboutToBeReset, this, &MatchesGraphicsManager::beginResetCategoryMatches);


    mTimer.start(TIMER_INTERVAL);

    beginResetMatches();
    endResetMatches();
}

void MatchesGraphicsManager::beginResetMatches() {
    mLoadedMatches.clear();
    mLoadedGroups.clear();
    mUnfinishedMatches.clear();
    mUnfinishedMatchesSet.clear();
    mUnfinishedMatchesPlayers.clear();
    mUnfinishedMatchesPlayersInv.clear();
    mUnpausedMatches.clear();
}

void MatchesGraphicsManager::endResetMatches() {
    loadBlocks();
}

void MatchesGraphicsManager::loadBlocks() {
    const auto &tournament = mStoreManager.getTournament();
    const auto &tatamis = tournament.getTatamis();
    if (!tatamis.containsTatami(mLocation))
        return;
    const auto &tatami = tatamis.at(mLocation);

    struct MatchInfo {
        CategoryId categoryId;
        MatchId matchId;
        MatchStatus status;
        std::optional<PlayerId> whitePlayer;
        std::optional<PlayerId> bluePlayer;
        bool osaekomi;
        bool bye;
    };

    std::vector<MatchInfo> newMatches;
    size_t newUnfinishedMatches = 0;

    while (mUnfinishedMatches.size() + newUnfinishedMatches < ROW_CAP) {
        if (mLoadedGroups.size() == tatami.groupCount())
            break;

        auto handle = tatami.getHandle(mLoadedGroups.size());
        mLoadedGroups.insert(handle.id);
        const auto &group = tatami.at(handle);

        for (const auto &p : group.getMatches()) {
            MatchInfo matchInfo;
            auto &category = tournament.getCategory(p.first);
            auto &match = category.getMatch(p.second);

            matchInfo.categoryId = p.first;
            matchInfo.matchId = p.second;
            matchInfo.status =  match.getStatus();
            matchInfo.whitePlayer = match.getWhitePlayer();
            matchInfo.bluePlayer = match.getBluePlayer();
            matchInfo.bye = match.isBye();
            matchInfo.osaekomi = match.getOsaekomi().has_value();

            if (!matchInfo.bye && matchInfo.status != MatchStatus::FINISHED)
                ++newUnfinishedMatches;

            newMatches.push_back(std::move(matchInfo));
        }
    }

    if (!newMatches.empty()) {
        for (const MatchInfo &matchInfo : newMatches) {
            auto loadingTime = mLoadedMatches.size();
            auto combinedId = std::make_pair(matchInfo.categoryId, matchInfo.matchId);
            mLoadedMatches[combinedId] = loadingTime;

            if (!matchInfo.bye && matchInfo.status != MatchStatus::FINISHED) {
                mUnfinishedMatches.push_back(std::make_tuple(matchInfo.categoryId, matchInfo.matchId, loadingTime));
                mUnfinishedMatchesSet.insert(combinedId);

                if (matchInfo.status == MatchStatus::UNPAUSED || matchInfo.osaekomi)
                    mUnpausedMatches.insert(combinedId);

                mUnfinishedMatchesPlayersInv[combinedId] = {matchInfo.whitePlayer, matchInfo.bluePlayer};
                if (matchInfo.whitePlayer)
                    mUnfinishedMatchesPlayers[*matchInfo.whitePlayer].insert(combinedId);
                if (matchInfo.bluePlayer)
                    mUnfinishedMatchesPlayers[*matchInfo.bluePlayer].insert(combinedId);
            }

        }
    }
}

