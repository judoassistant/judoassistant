#include <QHBoxLayout>
#include <QGraphicsView>

#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/graphics_items/tatami_text_graphics_item.hpp"
#include "ui/widgets/matches_widget.hpp"
#include "core/log.hpp"

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
        auto item = new TatamiTextGraphicsItem(i, MatchesGridGraphicsManager::GRID_WIDTH, MatchesGridGraphicsManager::VERTICAL_OFFSET);
        item->setPos(MatchesGridGraphicsManager::HORIZONTAL_OFFSET + i * MatchesGridGraphicsManager::GRID_WIDTH, 0);
        mScene->addItem(item);
        mItems.push_back(item);
    }
}

MatchesWidget::MatchesWidget(StoreManager &storeManager)
    : mStoreManager(storeManager)
{
    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &MatchesWidget::beginTournamentReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &MatchesWidget::endTournamentReset);

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
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // view->setMaximumWidth(UnallocatedBlockItem::WIDTH + PADDING*2 + 16);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    // mFixedScrollArea = new FixedScrollArea(300, mainSplitWidget);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(view);
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
    mTatamis.clear();
}

void MatchesWidget::endTatamiCountChange() {
    TatamiList &tatamis = mStoreManager.getTournament().getTatamis();

    for (size_t i = 0; i < tatamis.tatamiCount(); ++i) {
        int x = MatchesGridGraphicsManager::HORIZONTAL_OFFSET + i * MatchesGridGraphicsManager::GRID_WIDTH;
        int y = MatchesGridGraphicsManager::VERTICAL_OFFSET;
        TatamiLocation location{tatamis.getHandle(i)};

        auto tatami = std::make_unique<MatchesGraphicsManager>(mStoreManager, palette(), mScene, location, x, y);
        mTatamis.push_back(std::move(tatami));
    }

    mGrid->updateGrid(tatamis.tatamiCount());
}

MatchesGraphicsManager::MatchesGraphicsManager(StoreManager &storeManager, const QPalette &palette, QGraphicsScene *scene, TatamiLocation location, int x, int y)
    : mStoreManager(storeManager)
    , mPalette(palette)
    , mLocation(location)
    , mX(x)
    , mY(y)
    , mScene(scene)
    , mResettingMatches(false)
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
    if (mResettingMatches)
        return;

    mResettingMatches = true;
    mLoadedMatches.clear();
    mLoadedGroups.clear();
    mUnfinishedMatches.clear();
    mUnfinishedMatchesSet.clear();
    mUnfinishedMatchesPlayers.clear();
    mUnfinishedMatchesPlayersInv.clear();
    mUnpausedMatches.clear();
}

void MatchesGraphicsManager::endResetMatches() {
    loadBlocks(true);
    mResettingMatches = false;
}

void MatchesGraphicsManager::loadBlocks(bool forceReloadItems) {
    const auto &tournament = mStoreManager.getTournament();
    const auto &tatamis = tournament.getTatamis();
    if (!tatamis.containsTatami(mLocation))
        return;
    const auto &tatami = tatamis.at(mLocation);

    struct MatchInfo {
        CombinedId combinedId;
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

        for (const auto &combinedId : group.getMatches()) {
            MatchInfo matchInfo;
            auto &category = tournament.getCategory(combinedId.getCategoryId());
            auto &match = category.getMatch(combinedId.getMatchId());

            matchInfo.combinedId = combinedId;
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

    bool shouldReloadItems = forceReloadItems;

    if (mUnfinishedMatches.size() < ROW_CAP && newUnfinishedMatches > 0)
        shouldReloadItems = true;

    if (!newMatches.empty()) {
        for (const MatchInfo &matchInfo : newMatches) {
            auto loadingTime = mLoadedMatches.size();
            mLoadedMatches[matchInfo.combinedId] = loadingTime;

            if (!matchInfo.bye && matchInfo.status != MatchStatus::FINISHED) {
                mUnfinishedMatches.push_back(std::make_pair(matchInfo.combinedId, loadingTime));
                mUnfinishedMatchesSet.insert(matchInfo.combinedId);

                if (matchInfo.status == MatchStatus::UNPAUSED || matchInfo.osaekomi)
                    mUnpausedMatches.insert(matchInfo.combinedId);

                mUnfinishedMatchesPlayersInv[matchInfo.combinedId] = {matchInfo.whitePlayer, matchInfo.bluePlayer};
                if (matchInfo.whitePlayer)
                    mUnfinishedMatchesPlayers[*matchInfo.whitePlayer].insert(matchInfo.combinedId);
                if (matchInfo.bluePlayer)
                    mUnfinishedMatchesPlayers[*matchInfo.bluePlayer].insert(matchInfo.combinedId);
            }

        }
    }

    if (shouldReloadItems)
        reloadItems();
}

void MatchesGraphicsManager::changeMatches(CategoryId categoryId, const std::vector<MatchId> &matchIds) {
    bool didRemoveRows = false;
    bool shouldReloadItems = false;

    for (auto matchId : matchIds) {
        CombinedId combinedId(categoryId, matchId);
        auto it = mLoadedMatches.find(combinedId);
        if (it == mLoadedMatches.end())
            continue;

        const auto &category = mStoreManager.getTournament().getCategory(categoryId);
        const auto &match = category.getMatch(matchId);

        bool wasFinished = (mUnfinishedMatchesSet.find(combinedId) == mUnfinishedMatchesSet.end());
        bool isFinished = (match.isBye() || match.getStatus() == MatchStatus::FINISHED);

        if (match.getStatus() == MatchStatus::UNPAUSED || match.getOsaekomi().has_value())
            mUnpausedMatches.insert(combinedId);
        else
            mUnpausedMatches.erase(combinedId);

        if (!isFinished && !wasFinished) {
            // May need to update players
            auto it = mUnfinishedMatchesPlayersInv.find(combinedId);
            assert(it != mUnfinishedMatchesPlayersInv.end());
            auto prevWhitePlayer = it->second.first;
            auto prevBluePlayer = it->second.second;

            if (match.getWhitePlayer() != prevWhitePlayer) {
                if (prevWhitePlayer) {
                    auto it1 = mUnfinishedMatchesPlayers.find(*prevWhitePlayer);
                    assert(it1 != mUnfinishedMatchesPlayers.end());
                    auto &set = it1->second;
                    set.erase(combinedId);

                    if (set.empty())
                        mUnfinishedMatchesPlayers.erase(it1);
                }

                if (match.getWhitePlayer()) {
                    mUnfinishedMatchesPlayers[*(match.getWhitePlayer())].insert(combinedId);
                }
            }

            if (match.getBluePlayer() != prevBluePlayer) {
                if (prevBluePlayer) {
                    auto it1 = mUnfinishedMatchesPlayers.find(*prevBluePlayer);
                    assert(it1 != mUnfinishedMatchesPlayers.end());
                    auto &set = it1->second;
                    set.erase(combinedId);

                    if (set.empty())
                        mUnfinishedMatchesPlayers.erase(it1);
                }

                if (match.getBluePlayer()) {
                    mUnfinishedMatchesPlayers[*(match.getBluePlayer())].insert(combinedId);
                }
            }

            if (match.getWhitePlayer() != prevWhitePlayer || match.getBluePlayer() != prevBluePlayer) {
                mUnfinishedMatchesPlayersInv[combinedId] = {match.getWhitePlayer(), match.getBluePlayer()};
            }
        }
        else if (isFinished && !wasFinished) {
            size_t row = 0;
            for (const auto & p : mUnfinishedMatches) {
                if (p.first == combinedId)
                    break;
                ++row;
            }

            mUnfinishedMatches.erase(mUnfinishedMatches.begin() + row);
            mUnfinishedMatchesSet.erase(combinedId);

            auto it = mUnfinishedMatchesPlayersInv.find(combinedId);
            auto prevWhitePlayer = it->second.first;
            if (prevWhitePlayer) {
                auto it1 = mUnfinishedMatchesPlayers.find(*prevWhitePlayer);
                assert(it1 != mUnfinishedMatchesPlayers.end());
                auto & set = it1->second;
                set.erase(combinedId);

                if (set.empty())
                    mUnfinishedMatchesPlayers.erase(it1);
            }

            auto prevBluePlayer = it->second.second;
            if (prevBluePlayer) {
                auto it1 = mUnfinishedMatchesPlayers.find(*prevBluePlayer);
                assert(it1 != mUnfinishedMatchesPlayers.end());
                auto & set = it1->second;
                set.erase(combinedId);

                if (set.empty())
                    mUnfinishedMatchesPlayers.erase(it1);
            }
            mUnfinishedMatchesPlayersInv.erase(it);

            if (row < ROW_CAP)
                shouldReloadItems = true;
        }
        else if (!isFinished && wasFinished) {
            auto loadingTime = it->second;

            // Find the first position with a higher loading time (lower bound)
            auto pos = mUnfinishedMatches.begin();
            int row = 0;
            while (pos != mUnfinishedMatches.end() && pos->second < loadingTime) {
                ++pos;
                ++row;
            }

            mUnfinishedMatches.insert(pos, std::make_pair(combinedId, loadingTime));
            mUnfinishedMatchesSet.insert(combinedId);

            if (match.getWhitePlayer())
                mUnfinishedMatchesPlayers[*(match.getWhitePlayer())].insert(combinedId);
            if (match.getBluePlayer())
                mUnfinishedMatchesPlayers[*(match.getBluePlayer())].insert(combinedId);
            mUnfinishedMatchesPlayersInv[combinedId] = {match.getWhitePlayer(), match.getBluePlayer()};

            if (row < ROW_CAP)
                shouldReloadItems = true;
        }
    }

    if (didRemoveRows)
        loadBlocks(true);
    else if (shouldReloadItems)
        reloadItems();
    else {
        for (auto matchId : matchIds) {
            const CombinedId combinedId(categoryId, matchId);
            auto it = mItems.find(combinedId);
            if (it != mItems.end()) {
                MatchGraphicsItem *item = it->second;
                auto rect = item->boundingRect();
                item->update(rect);
            }
        }
    }
}

void MatchesGraphicsManager::changeTatamis(const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) {
    log_debug().field("blocks", blocks).msg("Changing locations");
    const auto &tournament = mStoreManager.getTournament();
    const auto &tatamis = tournament.getTatamis();
    if (!tatamis.containsTatami(mLocation))
        return;
    const auto &tatami = tatamis.at(mLocation);

    bool shouldReset = false;
    bool shouldLoad = false;

    for (BlockLocation location : locations) {
        if (mLocation != location.sequentialGroup.concurrentGroup.tatami) continue;

        if (mResettingMatches) {
            shouldReset = true;
            break;
        }

        auto handle = location.sequentialGroup.concurrentGroup.handle;
        if (mLoadedGroups.find(handle.id) != mLoadedGroups.end()) {
            shouldReset = true;
            break;
        }

        if (tatami.containsGroup(handle)) {
            if (tatami.getIndex(handle) < mLoadedGroups.size()) {
                shouldReset = true;
                break;
            }

            if (mUnfinishedMatches.size() < ROW_CAP) {
                shouldLoad = true;
            }
        }
    }

    if (shouldReset) {
        beginResetMatches();
        endResetMatches();
    }
    else if (shouldLoad) {
        loadBlocks();
    }
}

void MatchesGraphicsManager::changePlayers(const std::vector<PlayerId> &playerIds) {
    std::unordered_set<int> changedRows;

    for (auto playerId : playerIds) {
        auto it = mUnfinishedMatchesPlayers.find(playerId);
        if (it != mUnfinishedMatchesPlayers.end()) {
            for (auto combinedId : it->second) {
                auto it = mItems.find(combinedId);
                if (it != mItems.end()) {
                    MatchGraphicsItem *item = it->second;
                    item->update();
                }
            }
        }
    }
}

void MatchesGraphicsManager::beginResetCategoryMatches(const std::vector<CategoryId> &categoryIds) {
    const auto &tournament = mStoreManager.getTournament();

    for (auto categoryId : categoryIds) {
        const auto &category = tournament.getCategory(categoryId);
        for (const auto &match : category.getMatches()) {
            if (mLoadedMatches.find(match->getCombinedId()) != mLoadedMatches.end()) {
                beginResetMatches(); // Let the tatamiChanged call endResetMatches()
                return;
            }
        }
    }
}

void MatchesGraphicsManager::timerHit() {
    for (auto combinedId : mUnpausedMatches) {
        auto it = mItems.find(combinedId);
        if (it != mItems.end()) {
            MatchGraphicsItem *item = it->second;
            item->update();
        }
    }
}

void MatchesGraphicsManager::reloadItems() {
    for (auto &it : mItems) {
        MatchGraphicsItem *item = it.second;
        mScene->removeItem(item);
        delete item;
    }

    mItems.clear();

    int x = mX;
    int y = mY;

    for (size_t i = 0; i < mUnfinishedMatches.size() && i < ROW_CAP; ++i) {
        auto e = mUnfinishedMatches[i];
        const CombinedId &combinedId = e.first;

        QRect rect(x + MatchesGridGraphicsManager::HORIZONTAL_PADDING, y, MatchesGridGraphicsManager::GRID_WIDTH - MatchesGridGraphicsManager::HORIZONTAL_PADDING * 2, MatchesGridGraphicsManager::GRID_HEIGHT - MatchesGridGraphicsManager::BOTTOM_PADDING);
        auto item = new MatchGraphicsItem(mStoreManager, combinedId, rect);
        mScene->addItem(item);
        mItems[combinedId] = item;
        y += MatchesGridGraphicsManager::GRID_HEIGHT;
    }
}

