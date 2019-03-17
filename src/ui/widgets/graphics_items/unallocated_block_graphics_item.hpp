#pragma once

#include <QGraphicsItem>

#include "core/stores/match_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

class CategoryStore;

class UnallocatedBlockGraphicsItem : public QGraphicsItem {
public:
    static const int WIDTH = 200;
    static const int HEIGHT = 80;
    static const int PADDING = 3;

    UnallocatedBlockGraphicsItem(const CategoryStore &category, MatchType type);

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

