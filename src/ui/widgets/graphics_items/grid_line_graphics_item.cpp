#include "ui/widgets/graphics_items/grid_line_graphics_item.hpp"
#include "ui/widgets/new_tatamis_widget.hpp"
#include "ui/widgets/colors.hpp"
#include <QPainter>
// #include <QFont>

GridLineGraphicsItem::GridLineGraphicsItem(int minutes, int width)
    : mMinutes(minutes)
    , mWidth(width)
{}

QRectF GridLineGraphicsItem::boundingRect() const {
    return QRectF(0, -10, mWidth, 20);
}

void GridLineGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    constexpr int PADDING = 5;

    QRectF textRect(0, -10, GridGraphicsManager::HORIZONTAL_OFFSET-PADDING, 20);

    QFont font("Noto Sans");
    font.setCapitalization(QFont::AllUppercase);

    painter->setFont(font);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(COLOR_3);

    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, QObject::tr("%1 min").arg(mMinutes));

    painter->setPen(COLOR_4);
    painter->drawLine(GridGraphicsManager::HORIZONTAL_OFFSET, 0, mWidth - GridGraphicsManager::HORIZONTAL_OFFSET, 0);
}

