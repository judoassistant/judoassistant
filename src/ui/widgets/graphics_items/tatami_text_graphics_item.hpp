#pragma once

#include <QGraphicsItem>

class TatamiTextGraphicsItem : public QGraphicsItem {
public:
    TatamiTextGraphicsItem(int index, int width, int height);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
private:
    int mIndex;
    int mWidth;
    int mHeight;
};

