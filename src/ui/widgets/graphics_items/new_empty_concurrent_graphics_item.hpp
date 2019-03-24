#pragma once


#include "core/id.hpp"
#include "core/position_manager.hpp"
#include "core/stores/tatami/location.hpp"
#include <QGraphicsItem>

class StoreManager;
enum class MatchType;

class NewEmptyConcurrentGraphicsItem : public QGraphicsItem {
public:
    static constexpr int HEIGHT = 14;

    NewEmptyConcurrentGraphicsItem(StoreManager * storeManager, TatamiLocation tatami, size_t index);
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

