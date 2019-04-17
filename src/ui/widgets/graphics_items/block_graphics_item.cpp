#include <QApplication>
#include <QBitmap>
#include <QCursor>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QPainter>

#include "core/stores/match_store.hpp"
#include "ui/misc/judoassistant_mime.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/graphics_items/block_graphics_item.hpp"
#include "ui/widgets/graphics_items/sequential_graphics_item.hpp"
#include "ui/widgets/tatamis_widget.hpp"

BlockGraphicsItem::BlockGraphicsItem(StoreManager *storeManager, std::pair<CategoryId, MatchType> block, int height, SequentialGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mStoreManager(storeManager)
    , mBlock(block)
    , mHeight(height)
    , mWidth(parent->getWidth() - GridGraphicsManager::MARGIN)
{
    const auto &category = mStoreManager->getTournament().getCategory(block.first);
    mName = QString::fromStdString(category.getName());
    mMinutes = std::chrono::duration_cast<std::chrono::minutes>(category.expectedDuration(mBlock.second)).count();
    mEmpty = category.getMatches().empty();
    mStatus = category.getStatus(block.second);

    setCursor(Qt::OpenHandCursor);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF BlockGraphicsItem::boundingRect() const {
    // TODO: Try to use QRect instead of QRectF
    return QRectF(GridGraphicsManager::MARGIN, GridGraphicsManager::MARGIN, mWidth - 2*GridGraphicsManager::MARGIN, mHeight - GridGraphicsManager::MARGIN);
}

void BlockGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QPalette palette = (widget != nullptr ? widget->palette() : QApplication::palette());
    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(palette.color(QPalette::Dark));
    painter->setPen(pen);

    if (mStatus.startedMatches == 0 && mStatus.finishedMatches == 0) {
        painter->setBrush(palette.color(QPalette::Button).lighter(120));
    }
    else if (mStatus.startedMatches > 0 || mStatus.notStartedMatches > 0)
        painter->setBrush(COLOR_UI_BLOCK_STARTED);
    else
        painter->setBrush(COLOR_UI_BLOCK_FINISHED);

    QRectF rect = boundingRect();
    painter->drawRect(rect);

    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(palette.color(QPalette::ButtonText));
    painter->setPen(pen);

    auto offsetX = GridGraphicsManager::MARGIN + PADDING;
    auto offsetY = GridGraphicsManager::MARGIN + PADDING;
    auto innerWidth = static_cast<int>(boundingRect().width()) - 2 * PADDING;

    QRect nameRect(offsetX, offsetY, innerWidth, 20);
    QRect typeRect(offsetX + 2*PADDING, offsetY+20, innerWidth-2*PADDING, 20);
    QRect timeRect(offsetX + 2*PADDING, offsetY+40, innerWidth-2*PADDING, 20);

    painter->drawText(nameRect, Qt::AlignTop | Qt::AlignLeft, mName);

    // TODO: Rename knockout to elimination in code base
    QString type = (mBlock.second == MatchType::FINAL ? QObject::tr("Finals") : QObject::tr("Elimination"));
    painter->drawText(typeRect, Qt::AlignTop | Qt::AlignLeft, type);

    QString time;
    if (!mEmpty)
        time = QObject::tr("~ %1 min").arg(mMinutes);
    else
        time = QObject::tr("no matches");
    painter->drawText(timeRect, Qt::AlignTop | Qt::AlignLeft, time);
}

void BlockGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::ClosedHandCursor);
}

void BlockGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::OpenHandCursor);
}

void BlockGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    auto dist = QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length();
    if (dist < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(event->widget());
    auto *mime = new JudoassistantMime;
    drag->setMimeData(mime);

    // mime->setColorData(color);
    mime->setText(mName);
    mime->setBlock(mBlock.first, mBlock.second);

    QPixmap pixmap(mWidth, mHeight); // TODO: Fix offset
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    // painter.translate(15, 15);
    // painter.setRenderHint(QPainter::Antialiasing, false);
    paint(&painter, 0, 0);
    painter.end();

    pixmap.setMask(pixmap.createHeuristicMask());

    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(mWidth/2, mHeight/2));
    drag->exec();
}

