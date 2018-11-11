#pragma once

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QMetaObject>
#include "stores/tatami_store.hpp"
#include "store_managers/store_manager.hpp"
#include "widgets/misc/numerical_string_comparator.hpp"
#include <set>

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
    void tatamisChanged(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    void tatamisAdded(std::vector<size_t> indices);
    void tatamisAboutToBeErased(std::vector<size_t> indices);
    void categoriesAdded(std::vector<CategoryId> categoryId);
    void categoriesAboutToBeErased(std::vector<CategoryId> categoryId);
    void tournamentAboutToBeReset();
    void tournamentReset();
    void categoriesReset();

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

