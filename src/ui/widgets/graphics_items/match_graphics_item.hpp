#pragma once

#include <QGraphicsItem>

#include "core/id.hpp"

class MatchGraphicsItem : public QGraphicsItem {
public:
    MatchGraphicsItem(CategoryId category, MatchId match);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    CategoryId mCategory;
    MatchId mMatch;
};

