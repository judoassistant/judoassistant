#include <QBitmap>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QPainter>

#include "core/actions/set_tatami_location_action.hpp"
#include "ui/misc/judoassistant_mime.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/graphics_items/concurrent_graphics_item.hpp"
#include "ui/widgets/graphics_items/empty_concurrent_graphics_item.hpp"
#include "ui/widgets/graphics_items/sequential_graphics_item.hpp"
#include "ui/widgets/tatamis_widget.hpp"

ConcurrentGraphicsItem::ConcurrentGraphicsItem(StoreManager * storeManager, ConcurrentGroupLocation location)
    : mStoreManager(storeManager)
    , mLocation(location)
    , mDragOver(false)
{
    reloadBlocks();
    setAcceptDrops(true);
}

int ConcurrentGraphicsItem::getHeight() const {
    return mHeight;
}

QRectF ConcurrentGraphicsItem::boundingRect() const {
    constexpr int x = GridGraphicsManager::MARGIN;
    constexpr int y = GridGraphicsManager::MARGIN;
    constexpr int width = GridGraphicsManager::GRID_WIDTH - 2 * GridGraphicsManager::MARGIN;
    int height = getHeight() - GridGraphicsManager::MARGIN;
    return QRectF(x, y, width, height);
}

void ConcurrentGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);

    auto palette = widget->palette();
    if (mDragOver)
        pen.setColor(palette.color(QPalette::Highlight));
    else
        pen.setColor(palette.color(QPalette::AlternateBase));

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawRect(boundingRect());
}

void ConcurrentGraphicsItem::reloadBlocks() {
    for (auto * group : mSequentialGroups) {
        delete group;
    }

    mSequentialGroups.clear();

    const auto &tatamis = mStoreManager->getTournament().getTatamis();
    const auto &group = tatamis.at(mLocation);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(group.getExpectedDuration()).count();
    mHeight = minutes * GridGraphicsManager::GRID_HEIGHT / GridGraphicsManager::GRID_RESOLUTION - EmptyConcurrentGraphicsItem::HEIGHT;

    auto actualHeight = static_cast<int>(boundingRect().height());

    size_t offset = GridGraphicsManager::MARGIN;
    for (size_t i = 0; i < group.groupCount(); ++i) {
        auto * item = new SequentialGraphicsItem(mStoreManager, {mLocation, group.getHandle(i)}, actualHeight, this);
        item->setPos(offset, GridGraphicsManager::MARGIN);
        offset += item->getWidth();
        mSequentialGroups.push_back(item);
    }

    prepareGeometryChange();
    update();
}

ConcurrentGroupLocation ConcurrentGraphicsItem::getLocation() const {
    return mLocation;
}

void ConcurrentGraphicsItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    if (event->mimeData()->hasFormat("application/judoassistant-block") && mSequentialGroups.size() < ConcurrentBlockGroup::MAX_GROUP_COUNT) {
        event->setAccepted(true);
        mDragOver = true;
    }
    else {
        event->setAccepted(false);
    }

    update();
}

void ConcurrentGraphicsItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event) {
    mDragOver = false;
    update();
}

void ConcurrentGraphicsItem::dropEvent(QGraphicsSceneDragDropEvent *event) {
    const auto * mime = dynamic_cast<const JudoassistantMime*>(event->mimeData());
    auto block = mime->block();

    auto &tournament = mStoreManager->getTournament();
    auto &group = tournament.getTatamis().at(mLocation);

    BlockLocation location;
    location.sequentialGroup = group.generateLocation(mLocation, mSequentialGroups.size());
    location.pos = 0;

    mStoreManager->dispatch(std::make_unique<SetTatamiLocationAction>(block, location));
    mDragOver = false;
    update();
}

