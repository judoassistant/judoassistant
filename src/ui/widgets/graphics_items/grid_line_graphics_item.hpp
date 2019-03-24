#pragma once

#include <QGraphicsItem>

class GridLineGraphicsItem : public QGraphicsItem {
public:
    GridLineGraphicsItem(int minutes, int width);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
private:
    int mMinutes;
    int mWidth;
};

