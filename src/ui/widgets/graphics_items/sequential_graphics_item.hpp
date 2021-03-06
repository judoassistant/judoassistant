#pragma once

#include <QGraphicsItem>

#include "core/id.hpp"
#include "core/position_manager.hpp"
#include "core/stores/tatami/location.hpp"

class StoreManager;
class ConcurrentGraphicsItem;

class SequentialGraphicsItem : public QGraphicsItem {
public:
    SequentialGraphicsItem(StoreManager *storeManager, SequentialGroupLocation location, int height, ConcurrentGraphicsItem *parent);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    SequentialGroupLocation getLocation() const;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

    int getWidth() const;

private:
    StoreManager *mStoreManager;
    SequentialGroupLocation mLocation;
    int mHeight;
    size_t mBlockCount;
    bool mDragOver;
};

