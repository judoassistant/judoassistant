#include <QPainter>

#include "ui/widgets/graphics_items/tatami_text_graphics_item.hpp"
#include "ui/widgets/tatamis_widget.hpp"
#include "ui/widgets/colors.hpp"

TatamiTextGraphicsItem::TatamiTextGraphicsItem(int index)
    : mIndex(index)
{}

QRectF TatamiTextGraphicsItem::boundingRect() const {
    return QRectF(0, 0, GridGraphicsManager::GRID_WIDTH, GridGraphicsManager::VERTICAL_OFFSET);
}

void TatamiTextGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QFont font("Noto Sans");
    font.setCapitalization(QFont::AllUppercase);

    auto palette = widget->palette();

    painter->setFont(font);
    painter->setPen(palette.color(QPalette::Text));
    painter->setBrush(Qt::NoBrush);

    painter->drawText(boundingRect(), Qt::AlignCenter, QObject::tr("Tatami %1").arg(mIndex + 1));
}

