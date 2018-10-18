#pragma once

#include <QGraphicsView>
#include <QGraphicsItem>
#include "stores/tatami_store.hpp"
#include "store_handlers/qstore_handler.hpp"

class EmptyConcurrentBlockItem : public QGraphicsItem {
public:
    static const int WIDTH = 280;
    static const int HEIGHT = 20;

    EmptyConcurrentBlockItem(QStoreHandler * storeHandler, size_t tatamiIndex, size_t groupIndex);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

private:
    QStoreHandler *mStoreHandler;
    size_t mTatamiIndex;
    size_t mGroupIndex;
    bool mDragOver;
};

class BlockItem : public QGraphicsItem {
public:
    static const int WIDTH = 70;
    static const int PADDING = 5;

    BlockItem(QStoreHandler * storeHandler, CategoryId categoryId, MatchType type, QGraphicsItem *parent = nullptr);
    int getHeight() const;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QStoreHandler *mStoreHandler;
    CategoryStore *mCategory;
    MatchType mType;
    int mMatchCount;
};

class SequentialBlockItem : public QGraphicsItem {
public:
    static const int WIDTH = 80;
    static const int PADDING = 5;
    static const int BLOCK_MARGIN = 2;

    SequentialBlockItem(QStoreHandler * storeHandler, size_t tatamiIndex, PositionHandle concurrentHandle, PositionHandle handle, QGraphicsItem *parent = nullptr);
    int getHeight() const;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

private:
    QStoreHandler *mStoreHandler;
    size_t mTatamiIndex;
    PositionHandle mConcurrentHandle;
    PositionHandle mHandle;
    std::vector<std::pair<CategoryId, MatchType>> mBlocks;
    int mHeight;
    bool mDragOver;
};

class ConcurrentBlockItem : public QGraphicsItem {
public:
    static const int WIDTH = 280;
    static const int PADDING = 5;

    ConcurrentBlockItem(QStoreHandler * storeHandler, size_t tatamiIndex, PositionHandle handle);
    int getHeight() const;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reloadBlocks();
    PositionHandle getHandle() const;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

private:
    QStoreHandler *mStoreHandler;
    size_t mTatamiIndex;
    int mHeight;
    bool mDragOver;
    PositionHandle mHandle;
    std::vector<SequentialBlockItem*> mSequentialGroups;
};

class TatamiWidget : public QGraphicsView {
    Q_OBJECT
public:
    static const int WIDTH = 300;
    static const int PADDING = 10;
    static const int BLOCK_MARGIN = 2;

    TatamiWidget(QStoreHandler & storeHandler, size_t index, QWidget *parent = nullptr);

    void tatamisChanged(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    void tatamisReset();

private:
    void reloadBlocks();
    void shiftBlocks();

    QGraphicsScene *mScene;
    QStoreHandler *mStoreHandler;
    std::vector<EmptyConcurrentBlockItem*> mEmptyGroups;
    std::list<ConcurrentBlockItem*> mGroups;

    // todo: consider switching to lists.
    size_t mIndex;
};

