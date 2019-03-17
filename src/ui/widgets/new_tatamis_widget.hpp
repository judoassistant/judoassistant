#pragma once

#include <stack>

#include <QWidget>
#include <QHBoxLayout>
#include <QGraphicsScene>

#include "core/core.hpp"
#include "core/stores/tatami/location.hpp"
#include "ui/widgets/fixed_scroll_area.hpp"

// TODO: Display whenever two matches conflict with player
// TODO: Consider updating more efficiently when tatami count is changed
// TODO: Handle resizing when changing tatami count
class StoreManager;
class TatamiWidget;
class GridGraphicsItem;
class NewConcurrentGraphicsItem;
class NewEmptyConcurrentGraphicsItem;
enum class MatchType;

class GridGraphicsManager {
public:
    GridGraphicsManager(QGraphicsScene *mScene);

    static constexpr int GRID_RESOLUTION = 10; // The number of minutes per grid unit
    static constexpr int GRID_HEIGHT = 120; // The pixel size of a grid unit
    static constexpr int GRID_WIDTH = 300; // The pixel size of a grid unit
    static constexpr int VERTICAL_OFFSET = 80; // The amount of vertical space used for headers
    static constexpr int HORIZONTAL_OFFSET = 80; // The amount of vertical space used for ticks
    static constexpr int MARGIN = 2;

    void setMinutes(int minutes);
    void setTatamiCount(int tatamiCount);
    void setMinSize(int width, int height);

    void updateGrid(int tatamiCount, int minutes, int minWidth, int minHeight);

private:
    std::vector<QGraphicsItem*> mItems;

    QGraphicsScene *mScene;

    int mTatamiCount;
    int mMinutes;
    int mMinWidth;
    int mMinHeight;
};

class TatamiGraphicsManager {
public:
    TatamiGraphicsManager(StoreManager & storeManager, QGraphicsScene *scene, TatamiLocation location, int x, int y);

    void changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    void reloadBlocks();

    void clearBlocks();

    int getMinutes() const;

private:
    void shiftBlocks();

    std::vector<NewEmptyConcurrentGraphicsItem*> mEmptyGroups;
    std::list<NewConcurrentGraphicsItem*> mGroups;
    StoreManager & mStoreManager;
    QGraphicsScene *mScene;
    TatamiLocation mLocation;
    int mX;
    int mY;
    int mMinutes;
};


class NewTatamisWidget : public QWidget {
    Q_OBJECT
public:
    NewTatamisWidget(StoreManager &storeManager);

private:
    void beginTournamentReset();
    void endTournamentReset();

    void beginTatamiCountChange();
    void endTatamiCountChange();

    void changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    void endCategoriesReset();

    StoreManager &mStoreManager;
    QGraphicsScene *mScene;
    GridGraphicsManager *mGrid;
    std::vector<TatamiGraphicsManager> mTatamis;
    std::stack<QMetaObject::Connection> mConnections;
};

