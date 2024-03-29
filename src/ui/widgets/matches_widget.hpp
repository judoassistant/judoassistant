#pragma once

#include <stack>
#include <vector>
#include <QWidget>
#include <QTimer>

#include "core/id.hpp"
#include "core/stores/tatami/location.hpp"
#include "core/stores/match_store.hpp"
#include "ui/widgets/graphics_items/match_graphics_item.hpp"

class StoreManager;
class QGraphicsScene;
class QGraphicsItem;

class MatchesGridGraphicsManager {
public:
    static constexpr int HORIZONTAL_OFFSET = 10;
    static constexpr int VERTICAL_OFFSET = 80;
    static constexpr int BOTTOM_PADDING = 10;
    static constexpr int HORIZONTAL_PADDING = 25;
    static constexpr int GRID_HEIGHT = MatchGraphicsItem::HEIGHT_HINT + BOTTOM_PADDING;
    static constexpr int GRID_WIDTH = MatchGraphicsItem::WIDTH_HINT + HORIZONTAL_PADDING * 2;

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
    std::unordered_map<CombinedId, size_t> mLoadedMatches; // Matches loaded and loading time
    std::unordered_set<PositionId> mLoadedGroups; // Blocks loaded

    std::deque<std::pair<CombinedId, size_t>> mUnfinishedMatches; // Unfinished (and loaded) matches and loading time
    std::unordered_set<CombinedId> mUnfinishedMatchesSet;

    std::unordered_map<PlayerId, std::unordered_set<CombinedId>> mUnfinishedMatchesPlayers;
    std::unordered_map<CombinedId, std::pair<std::optional<PlayerId>, std::optional<PlayerId>>> mUnfinishedMatchesPlayersInv;
    std::unordered_set<CombinedId> mUnpausedMatches;

    std::unordered_map<CombinedId, MatchGraphicsItem*> mItems;
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

