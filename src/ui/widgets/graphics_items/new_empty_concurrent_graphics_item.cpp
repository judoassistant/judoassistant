#include <QBitmap>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QPainter>

#include "core/actions/tatami_actions.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/misc/judoassistant_mime.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/graphics_items/new_empty_concurrent_graphics_item.hpp"
#include "ui/widgets/new_tatamis_widget.hpp"

NewEmptyConcurrentGraphicsItem::NewEmptyConcurrentGraphicsItem(StoreManager * storeManager, TatamiLocation tatami, size_t index)
    : mStoreManager(storeManager)
    , mTatami(tatami)
    , mIndex(index)
    , mDragOver(false)
{
    setAcceptDrops(true);
}

QRectF NewEmptyConcurrentGraphicsItem::boundingRect() const {
    constexpr int x = GridGraphicsManager::MARGIN;
    constexpr int y = GridGraphicsManager::MARGIN;
    constexpr int width = GridGraphicsManager::GRID_WIDTH - 2 * GridGraphicsManager::MARGIN;
    constexpr int height = HEIGHT - GridGraphicsManager::MARGIN;
    return QRectF(x, y, width, height);
}

void NewEmptyConcurrentGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);

    if (mDragOver)
        pen.setColor(COLOR_8);
    else
        pen.setColor(COLOR_3);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawRect(boundingRect());
}

void NewEmptyConcurrentGraphicsItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    if (event->mimeData()->hasFormat("application/judoassistant-block")) {
        event->setAccepted(true);
        mDragOver = true;
    }
    else {
        event->setAccepted(false);
    }

    update();
}

void NewEmptyConcurrentGraphicsItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event) {
    mDragOver = false;
    update();
}

void NewEmptyConcurrentGraphicsItem::dropEvent(QGraphicsSceneDragDropEvent *event) {
    const auto * mime = dynamic_cast<const JudoassistantMime*>(event->mimeData());
    auto block = mime->block();
    auto &tournament = mStoreManager->getTournament();
    auto &tatamis = tournament.getTatamis();
    TatamiStore &tatami = tatamis.at(mTatami);

    BlockLocation location;
    location.sequentialGroup = tatami.generateLocation(mTatami, mIndex);
    location.pos = 0;

    mStoreManager->dispatch(std::make_unique<SetTatamiLocationAction>(block, location));
    mDragOver = false;
    update();
}

