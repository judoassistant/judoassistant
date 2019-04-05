#include "ui/widgets/graphics_items/grid_line_graphics_item.hpp"
#include "ui/widgets/tatamis_widget.hpp"
#include "ui/widgets/colors.hpp"
#include <QPainter>
#include "core/log.hpp"
// #include <QFont>

GridLineGraphicsItem::GridLineGraphicsItem(int minutes, int width)
    : mMinutes(minutes)
    , mWidth(width)
{
    setZValue(-1);
}

QRectF GridLineGraphicsItem::boundingRect() const {
    return QRectF(0, -10, mWidth, 20);
}

void GridLineGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    constexpr int PADDING = 5;

    QRectF textRect(0, -10, GridGraphicsManager::HORIZONTAL_OFFSET-PADDING, 20);

    QFont font("Noto Sans");
    font.setCapitalization(QFont::AllUppercase);

    auto palette = widget->palette();
    // log_debug().field("color", widget->palette().color(QPalette::Base).name().toStdString()).msg("Painting grid line");

    painter->setFont(font);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(palette.color(QPalette::WindowText));


    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, QObject::tr("%1 min").arg(mMinutes));

    painter->setPen(palette.color(QPalette::AlternateBase));
    painter->drawLine(GridGraphicsManager::HORIZONTAL_OFFSET, 0, mWidth - GridGraphicsManager::HORIZONTAL_OFFSET, 0);
}

