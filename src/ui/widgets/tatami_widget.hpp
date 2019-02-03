#pragma once

#include <QGraphicsView>
#include <QGraphicsItem>

#include "core/id.hpp"
#include "core/position_manager.hpp"
#include "core/stores/tatami/location.hpp"

class TatamiLocation;
class StoreManager;
enum class MatchType;

// TODO: Disable dragging for already started blocks
class EmptyConcurrentBlockItem : public QGraphicsItem {
public:
    static const int WIDTH = 280;
    static const int HEIGHT = 20;

    EmptyConcurrentBlockItem(StoreManager * storeManager, TatamiLocation tatami, size_t index);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

private:
    StoreManager *mStoreManager;
    TatamiLocation mTatami;
    size_t mIndex;
    bool mDragOver;
};

class BlockItem : public QGraphicsItem {
public:
    static const int WIDTH = 80;
    static const int PADDING = 5;

    BlockItem(StoreManager * storeManager, std::pair<CategoryId, MatchType> block, QGraphicsItem *parent = nullptr);
    int getHeight() const;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    StoreManager *mStoreManager;
    std::pair<CategoryId, MatchType> mBlock;
    int mMatchCount;
    QString mName;
};

class SequentialBlockItem : public QGraphicsItem {
public:
    static const int WIDTH = 90;
    static const int PADDING = 5;
    static const int BLOCK_MARGIN = 2;

    SequentialBlockItem(StoreManager * storeManager, SequentialGroupLocation location, QGraphicsItem *parent = nullptr);
    int getHeight() const;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

private:
    StoreManager *mStoreManager;
    SequentialGroupLocation mLocation;

    std::vector<std::pair<CategoryId, MatchType>> mBlocks;
    int mHeight;
    bool mDragOver;
};

class ConcurrentBlockItem : public QGraphicsItem {
public:
    static const int WIDTH = 280;
    static const int PADDING = 5;

    ConcurrentBlockItem(StoreManager * storeManager, ConcurrentGroupLocation location);
    int getHeight() const;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reloadBlocks();
    ConcurrentGroupLocation getLocation() const;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

private:
    StoreManager *mStoreManager;
    ConcurrentGroupLocation mLocation;
    int mHeight;
    bool mDragOver;
    std::vector<SequentialBlockItem*> mSequentialGroups;
};

class TatamiWidget : public QGraphicsView {
    Q_OBJECT
public:
    static const int WIDTH = 300;
    static const int PADDING = 10;
    static const int BLOCK_MARGIN = 2;

    TatamiWidget(StoreManager & storeManager, TatamiLocation location, QWidget *parent = nullptr);

    void changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    void endResetTatamis();

private:
    void reloadBlocks();
    void shiftBlocks();

    StoreManager *mStoreManager;
    TatamiLocation mLocation;
    QGraphicsScene *mScene;
    std::vector<EmptyConcurrentBlockItem*> mEmptyGroups;
    std::list<ConcurrentBlockItem*> mGroups;
};

