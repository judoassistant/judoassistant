#pragma once

#include <QGraphicsItem>

#include "core/id.hpp"
#include "core/position_manager.hpp"
#include "core/stores/tatami/location.hpp"
#include "core/stores/category_store.hpp"

class StoreManager;
class NewConcurrentGraphicsItem;
class NewSequentialGraphicsItem;
enum class MatchType;

class NewBlockGraphicsItem : public QGraphicsItem {
public:
    static constexpr int PADDING = 3;

    NewBlockGraphicsItem(StoreManager *storeManager, std::pair<CategoryId, MatchType> block, int height, NewSequentialGraphicsItem *parent);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    StoreManager *mStoreManager;
    std::pair<CategoryId, MatchType> mBlock;
    int mHeight;
    int mWidth;
    int mMinutes;
    QString mName;
    CategoryStatus mStatus;
};

