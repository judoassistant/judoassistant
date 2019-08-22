#pragma once

#include <QGraphicsItem>

#include "core/id.hpp"

class StoreManager;

class MatchGraphicsItem : public QGraphicsItem {
public:
    MatchGraphicsItem(StoreManager &storeManager, const QPalette &palette, CategoryId categoryId, MatchId matchId, QRect rect, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    const StoreManager &mStoreManager;
    const QPalette &mPalette;
    CategoryId mCategoryId;
    MatchId mMatchId;
    QRect mRect;
};

