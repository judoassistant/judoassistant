#include <QBitmap>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QPainter>

#include "core/actions/tatami_actions.hpp"
#include "core/log.hpp"
#include "ui/misc/judoassistant_mime.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/graphics_items/new_concurrent_graphics_item.hpp"
#include "ui/widgets/graphics_items/new_empty_concurrent_graphics_item.hpp"
#include "ui/widgets/graphics_items/new_sequential_graphics_item.hpp"
#include "ui/widgets/new_tatamis_widget.hpp"

NewConcurrentGraphicsItem::NewConcurrentGraphicsItem(StoreManager * storeManager, ConcurrentGroupLocation location)
    : mStoreManager(storeManager)
    , mLocation(location)
    , mDragOver(false)
{
    reloadBlocks();
    setAcceptDrops(true);
}

int NewConcurrentGraphicsItem::getHeight() const {
    return mHeight;
}

QRectF NewConcurrentGraphicsItem::boundingRect() const {
    constexpr int x = GridGraphicsManager::MARGIN;
    constexpr int y = GridGraphicsManager::MARGIN;
    constexpr int width = GridGraphicsManager::GRID_WIDTH - 2 * GridGraphicsManager::MARGIN;
    int height = getHeight() - GridGraphicsManager::MARGIN;
    return QRectF(x, y, width, height);
}

void NewConcurrentGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
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

void NewConcurrentGraphicsItem::reloadBlocks() {
    for (auto * group : mSequentialGroups) {
        delete group;
    }

    mSequentialGroups.clear();

    const auto &tatamis = mStoreManager->getTournament().getTatamis();
    const auto &group = tatamis.at(mLocation);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(group.getExpectedDuration()).count();
    mHeight = minutes * GridGraphicsManager::GRID_HEIGHT / GridGraphicsManager::GRID_RESOLUTION - NewEmptyConcurrentGraphicsItem::HEIGHT;

    auto actualHeight = static_cast<int>(boundingRect().height());

    size_t offset = GridGraphicsManager::MARGIN;
    for (size_t i = 0; i < group.groupCount(); ++i) {
        auto * item = new NewSequentialGraphicsItem(mStoreManager, {mLocation, group.getHandle(i)}, actualHeight, this);
        item->setPos(offset, GridGraphicsManager::MARGIN);
        offset += item->getWidth();
        mSequentialGroups.push_back(item);
    }

    prepareGeometryChange();
    update();
}

ConcurrentGroupLocation NewConcurrentGraphicsItem::getLocation() const {
    return mLocation;
}

void NewConcurrentGraphicsItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    if (event->mimeData()->hasFormat("application/judoassistant-block")) {
        event->setAccepted(true);
        mDragOver = true;
    }
    else {
        event->setAccepted(false);
    }

    update();
}

void NewConcurrentGraphicsItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event) {
    mDragOver = false;
    update();
}

void NewConcurrentGraphicsItem::dropEvent(QGraphicsSceneDragDropEvent *event) {
    // const auto * mime = dynamic_cast<const JudoassistantMime*>(event->mimeData());
    // auto block = mime->block();

    // auto &tournament = mStoreManager->getTournament();
    // auto &group = tournament.getTatamis().at(mLocation);

    // BlockLocation location;
    // location.sequentialGroup = group.generateLocation(mLocation, mSequentialGroups.size());
    // location.pos = 0;

    // mStoreManager->dispatch(std::make_unique<SetTatamiLocationAction>(block, location));
    mDragOver = false;
    update();
}

