#include "ui/widgets/graphics_items/sequential_graphics_item.hpp"

#include <QBitmap>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QPainter>

#include "core/actions/tatami_actions.hpp"
#include "core/log.hpp"
#include "core/stores/category_store.hpp"
#include "ui/misc/judoassistant_mime.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/graphics_items/block_graphics_item.hpp"
#include "ui/widgets/graphics_items/concurrent_graphics_item.hpp"
#include "ui/widgets/graphics_items/sequential_graphics_item.hpp"
#include "ui/widgets/tatamis_widget.hpp"

SequentialGraphicsItem::SequentialGraphicsItem(StoreManager *storeManager, SequentialGroupLocation location, int height, ConcurrentGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mStoreManager(storeManager)
    , mLocation(location)
    , mHeight(height)
    , mDragOver(false)
{
    setAcceptDrops(true);

    const auto &tournament = mStoreManager->getTournament();
    const auto &tatamis = tournament.getTatamis();
    const auto &group = tatamis.at(mLocation);
    auto expectedDuration = group.getExpectedDuration();

    int heightTotal = static_cast<int>(boundingRect().height()) - GridGraphicsManager::MARGIN; // Total shared height for all blocks
    int heightAccSum = 0;

    mBlockCount = group.blockCount();

    size_t offset = GridGraphicsManager::MARGIN;
    for (size_t i = 0; i < group.blockCount(); ++i) {
        auto block = group.at(i);

        const auto &category = tournament.getCategory(block.first);
        int height;
        if (i != group.blockCount() - 1) // Let the last item fill out the rest of the height
            height = heightTotal * category.expectedDuration(block.second) / expectedDuration;
        else
            height = heightTotal - heightAccSum;

        auto *item = new BlockGraphicsItem(mStoreManager, block, height, this);
        item->setPos(GridGraphicsManager::MARGIN, offset);
        offset += height;
        heightAccSum += height;
    }
}

int SequentialGraphicsItem::getWidth() const {
    auto totalWidth = GridGraphicsManager::GRID_WIDTH - 3 * GridGraphicsManager::MARGIN; // Include margin to the right
    return totalWidth / 3;
}

QRectF SequentialGraphicsItem::boundingRect() const {
    auto height = mHeight - 2 * GridGraphicsManager::MARGIN;
    auto width = getWidth() - GridGraphicsManager::MARGIN;
    return QRectF(GridGraphicsManager::MARGIN, GridGraphicsManager::MARGIN, width, height);
}

void SequentialGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);

    if (mDragOver)
        pen.setColor(COLOR_8);
    else
        pen.setColor(COLOR_4);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->setBrush(Qt::white);

    painter->drawRect(boundingRect());
}

SequentialGroupLocation SequentialGraphicsItem::getLocation() const {
    return mLocation;
}

void SequentialGraphicsItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    if (event->mimeData()->hasFormat("application/judoassistant-block")) {
        event->setAccepted(true);
        mDragOver = true;
    }
    else {
        event->setAccepted(false);
    }

    update();
}

void SequentialGraphicsItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event) {
    mDragOver = false;
    update();
}

void SequentialGraphicsItem::dropEvent(QGraphicsSceneDragDropEvent *event) {
    const auto * mime = dynamic_cast<const JudoassistantMime*>(event->mimeData());
    auto block = mime->block();

    BlockLocation location;
    location.sequentialGroup = mLocation;
    location.pos = mBlockCount;

    mDragOver = false;
    update();
    mStoreManager->dispatch(std::make_unique<SetTatamiLocationAction>(block, location));
}

