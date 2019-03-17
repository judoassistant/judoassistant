#pragma once

#include <QGraphicsItem>

class TatamiTextGraphicsItem : public QGraphicsItem {
public:
    TatamiTextGraphicsItem(int index);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
private:
    int mIndex;
};

