#pragma once

#include <QGraphicsItem>

#include "core/id.hpp"
#include "core/position_manager.hpp"
#include "core/stores/tatami/location.hpp"

class NewConcurrentGraphicsItem : public QGraphicsItem {
public:
    NewConcurrentGraphicsItem(StoreManager * storeManager, ConcurrentGroupLocation location);
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
    // std::vector<SequentialBlockItem*> mSequentialGroups;
};

