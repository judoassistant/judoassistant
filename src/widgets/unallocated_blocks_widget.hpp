#pragma once

#include <set>
#include <map>
#include <stack>

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QMetaObject>

#include "id.hpp"
#include "widgets/misc/numerical_string_comparator.hpp"
#include "stores/match_store.hpp"
#include "stores/tatami/tatami_location.hpp"

class StoreManager;
class CategoryStore;

class UnallocatedBlockItem : public QGraphicsItem
{
public:
    static const int WIDTH = 300;
    static const int HEIGHT = 30;
    static const int PADDING = 5;

    UnallocatedBlockItem(const CategoryStore &category, MatchType type);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    const CategoryStore *mCategory;
    MatchType mType;
};

class BlockComparator {
public:
    BlockComparator(const BlockComparator &other) = default;
    BlockComparator(const TournamentStore &tournament);
    bool operator()(const std::pair<CategoryId, MatchType>, const std::pair<CategoryId, MatchType>) const;
private:
    const TournamentStore *mTournament;
    NumericalStringComparator mComp;
};

class UnallocatedBlocksWidget : public QGraphicsView {
    Q_OBJECT
public:
    static const int ITEM_MARGIN = 3;

    UnallocatedBlocksWidget(StoreManager & storeManager, QWidget *parent = 0);

public slots:
    void endAddCategories(std::vector<CategoryId> categoryIds);
    void beginEraseCategories(std::vector<CategoryId> categoryIds);
    void changeCategories(std::vector<CategoryId> categoryIds);
    void beginTournamentReset();
    void endTournamentReset();
    void endCategoriesReset();

private:
    void reloadBlocks();
    void shiftBlocks();
    bool insertBlock(const CategoryStore &category, MatchType type);
    bool eraseBlock(CategoryId id, MatchType type);
    bool eraseBlock(const CategoryStore &category, MatchType type);

    QGraphicsScene *mScene;
    StoreManager & mStoreManager;
    std::set<std::pair<CategoryId, MatchType>, BlockComparator> mBlocks;
    std::map<std::pair<CategoryId, MatchType>, UnallocatedBlockItem*> mBlockItems;
    std::stack<QMetaObject::Connection> mConnections;
};

