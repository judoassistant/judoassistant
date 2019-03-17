#pragma once

#include <QGraphicsItem>

#include "core/id.hpp"
#include "core/position_manager.hpp"
#include "core/stores/tatami/location.hpp"

class StoreManager;

class NewSequentialGraphicsItem : public QGraphicsItem {
public:
    NewSequentialGraphicsItem(StoreManager *storeManager, SequentialGroupLocation location, int height);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reloadBlocks();
    SequentialGroupLocation getLocation() const;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

private:
    StoreManager *mStoreManager;
    SequentialGroupLocation mLocation;
    int mHeight;
    bool mDragOver;
    // std::vector<SequentialBlockItem*> mSequentialGroups;
};

