#pragma once

#include <stack>
#include <vector>
#include <QWidget>
#include <QTimer>

#include "core/id.hpp"
#include "core/stores/tatami/location.hpp"
#include "core/stores/match_store.hpp"
#include "ui/widgets/graphics_items/match_graphics_item.hpp"
#include "ui/models/match_card.hpp"

class StoreManager;
class QGraphicsScene;
class QGraphicsItem;

class MatchesGridGraphicsManager {
public:
    static constexpr int HORIZONTAL_OFFSET = 10;
    static constexpr int VERTICAL_OFFSET = 80;
    static constexpr int PADDING = 10;
    static constexpr int GRID_HEIGHT = 160 + PADDING;
    static constexpr int GRID_WIDTH = 350 + PADDING * 2;

    MatchesGridGraphicsManager(QGraphicsScene *scene);
    void updateGrid(unsigned int tatamiCount);

private:
    QGraphicsScene *mScene;

    std::vector<QGraphicsItem*> mItems;
};

class MatchesGraphicsManager : public QObject {
    Q_OBJECT
public:
    static constexpr auto TIMER_INTERVAL = std::chrono::milliseconds(1000);
    static constexpr auto ROW_CAP = 10;

    MatchesGraphicsManager(StoreManager &storeManager, const QPalette &palette, QGraphicsScene *scene, TatamiLocation location, int x, int y);

protected:
    void changeMatches(CategoryId categoryId, const std::vector<MatchId> &matchIds);
    void changeTatamis(const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks);
    void changePlayers(const std::vector<PlayerId> &playerIds);
    void timerHit();
    void beginResetCategoryMatches(const std::vector<CategoryId> &categoryIds);

    void beginResetMatches();
    void endResetMatches();
    void loadBlocks(bool forceReloadItems = false);
    void reloadItems();

private:
    StoreManager &mStoreManager;
    const QPalette &mPalette;
    TatamiLocation mLocation;
    int mX;
    int mY;
    QGraphicsScene *mScene;

    bool mResettingMatches;
    QTimer mTimer;
    std::unordered_map<std::pair<CategoryId,MatchId>, size_t> mLoadedMatches; // Matches loaded and loading time
    std::unordered_set<PositionId> mLoadedGroups; // Blocks loaded

    std::deque<std::tuple<CategoryId, MatchId, size_t>> mUnfinishedMatches; // Unfinished (and loaded) matches and loading time
    std::unordered_set<std::pair<CategoryId, MatchId>> mUnfinishedMatchesSet;

    std::unordered_map<PlayerId, std::unordered_set<std::pair<CategoryId, MatchId>>> mUnfinishedMatchesPlayers;
    std::unordered_map<std::pair<CategoryId, MatchId>, std::pair<std::optional<PlayerId>, std::optional<PlayerId>>> mUnfinishedMatchesPlayersInv;
    std::unordered_set<std::pair<CategoryId, MatchId>> mUnpausedMatches;

    std::vector<MatchGraphicsItem*> mItems;
    std::unordered_map<std::pair<CategoryId, MatchId>, std::vector<MatchGraphicsItem*>::iterator> mItemMap;
};

class MatchesWidget : public QWidget {
    Q_OBJECT
public:
    MatchesWidget(StoreManager &storeManager);

private:
    void beginTournamentReset();
    void endTournamentReset();

    void beginTatamiCountChange();
    void endTatamiCountChange();

    StoreManager &mStoreManager;
    QGraphicsScene *mScene;
    std::vector<std::unique_ptr<MatchesGraphicsManager>> mTatamis;
    std::stack<QMetaObject::Connection> mConnections;
    MatchesGridGraphicsManager *mGrid;
};
