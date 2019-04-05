#include <QBitmap>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QMetaObject>
#include <QPainter>
#include <QPen>
#include <QCursor>
#include <QApplication>
#include <QWidget>

#include "core/stores/category_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/graphics_items/unallocated_block_graphics_item.hpp"
#include "ui/misc/judoassistant_mime.hpp"
#include "core/log.hpp"

UnallocatedBlockGraphicsItem::UnallocatedBlockGraphicsItem(const CategoryStore &category, MatchType type)
    : mCategory(&category)
    , mType(type)
{
    setCursor(Qt::OpenHandCursor);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF UnallocatedBlockGraphicsItem::boundingRect() const {
    return QRectF(0, 0, WIDTH, HEIGHT);
}

void UnallocatedBlockGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QPalette palette = (widget != nullptr ? widget->palette() : QApplication::palette());

    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(palette.color(QPalette::Dark));
    painter->setPen(pen);

    const auto &categoryStatus = mCategory->getStatus(mType);
    if (categoryStatus.startedMatches == 0 && categoryStatus.finishedMatches == 0) {
        painter->setBrush(palette.color(QPalette::Button).lighter(120)); // TODO: Setup colors for different states
    }
    else if (categoryStatus.startedMatches > 0 || categoryStatus.notStartedMatches > 0)
        painter->setBrush(palette.color(QPalette::Button).lighter(120));
    else
        painter->setBrush(palette.color(QPalette::Button).lighter(120));

    QRect rect(0, 0, WIDTH, HEIGHT);
    painter->drawRect(rect);

    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(palette.color(QPalette::ButtonText));
    painter->setPen(pen);

    QRect titleRect(PADDING, PADDING, WIDTH-PADDING*2, 20);
    QRect typeRect(PADDING*5, 20+PADDING, WIDTH-PADDING*6, 20);
    QRect timeRect(PADDING*5, 40+PADDING, WIDTH-PADDING*6, 20);

    QString title = QString::fromStdString(mCategory->getName());
    painter->drawText(titleRect, Qt::AlignTop | Qt::AlignLeft, title);

    QString type = (mType == MatchType::FINAL ? QObject::tr("Finals") : QObject::tr("Elimination"));
    painter->drawText(typeRect, Qt::AlignTop | Qt::AlignLeft, type);

    unsigned int minutes = std::chrono::duration_cast<std::chrono::minutes>(mCategory->expectedDuration(mType)).count();
    QString time = QObject::tr("~ %1 min").arg(minutes);
    painter->drawText(timeRect, Qt::AlignTop | Qt::AlignLeft, time);
}

void UnallocatedBlockGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::ClosedHandCursor);
}

void UnallocatedBlockGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    auto dist = QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length();
    if (dist < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(event->widget());
    auto *mime = new JudoassistantMime;
    drag->setMimeData(mime);

    // mime->setColorData(color);
    mime->setText(QString::fromStdString(mCategory->getName(mType)));
    mime->setBlock(mCategory->getId(), mType);

    QPixmap pixmap(WIDTH, HEIGHT);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    // painter.translate(15, 15);
    painter.setRenderHint(QPainter::Antialiasing, false);
    paint(&painter, 0, 0);
    painter.end();

    pixmap.setMask(pixmap.createHeuristicMask());

    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(WIDTH/2, HEIGHT/2));
    drag->exec();
}

void UnallocatedBlockGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::OpenHandCursor);
}
