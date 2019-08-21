#pragma once

#include <stack>
#include <vector>
#include <QWidget>

#include "core/stores/tatami/location.hpp"

class StoreManager;
class QGraphicsScene;
class QGraphicsItem;

class MatchesGridGraphicsManager {
public:
    static constexpr int HORIZONTAL_OFFSET = 10;
    static constexpr int VERTICAL_OFFSET = 80;
    static constexpr int GRID_WIDTH = 300;

    MatchesGridGraphicsManager(QGraphicsScene *scene);
    void updateGrid(unsigned int tatamiCount);

private:
    QGraphicsScene *mScene;

    std::vector<QGraphicsItem*> mItems;
};

class MatchesGraphicsManager {
public:
    MatchesGraphicsManager(StoreManager &storeManager, QGraphicsScene *scene, TatamiLocation location, int x, int y);
private:
};

class NewMatchesWidget : public QWidget {
    Q_OBJECT
public:
    NewMatchesWidget(StoreManager &storeManager);

private:
    void beginTournamentReset();
    void endTournamentReset();

    void beginTatamiCountChange();
    void endTatamiCountChange();

    StoreManager &mStoreManager;
    QGraphicsScene *mScene;
    std::vector<MatchesGraphicsManager> mTatamis;
    std::stack<QMetaObject::Connection> mConnections;
    MatchesGridGraphicsManager *mGrid;
};
