#include <algorithm>

#include <QApplication>
#include <QDrag>
#include <QBitmap>
#include <QGraphicsSceneDragDropEvent>

#include "stores/category_store.hpp"
#include "stores/qtournament_store.hpp"
#include "store_managers/store_manager.hpp"
#include "widgets/misc/judoassistant_mime.hpp"
#include "widgets/tatami_widget.hpp"
#include "actions/tatami_actions.hpp"
#include "widgets/colors.hpp"

TatamiWidget::TatamiWidget(StoreManager & storeManager, size_t index, QWidget *parent)
    : QGraphicsView(parent)
    , mStoreManager(&storeManager)
    , mIndex(index)
{
    QTournamentStore & tournament = mStoreManager->getTournament();

    connect(&tournament, &QTournamentStore::tatamisChanged, this, &TatamiWidget::tatamisChanged);
    connect(&tournament, &QTournamentStore::categoriesReset, this, &TatamiWidget::tatamisReset);

    mScene = new QGraphicsScene(this);
    mScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    mScene->setSceneRect(0, 0, WIDTH, 1300);
    setScene(mScene);
    setCacheMode(CacheNone);
    setViewportUpdateMode(MinimalViewportUpdate);
    setRenderHint(QPainter::Antialiasing, false);
    setMinimumSize(WIDTH+2, 1300);
    setMaximumWidth(WIDTH+2);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    reloadBlocks();
}

void TatamiWidget::tatamisChanged(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {
    std::unordered_set<PositionId> affectedIds;
    for (TatamiLocation loc : locations) {
        if (loc.tatamiIndex != mIndex) continue;

        affectedIds.insert(loc.concurrentGroup.id);
    }

    if (affectedIds.empty())
        return;

    auto & tatami = mStoreManager->getTournament().getTatamis()[mIndex];

    for (auto i = mGroups.begin(); i != mGroups.end();) {
        auto next = std::next(i);
        if (!tatami.containsGroup((*i)->getHandle())) {
            mScene->removeItem(*i);
            delete (*i);
            mGroups.erase(i);
        }

        i = next;
    }

    // There is always an empty blocks at the top
    size_t offset = EmptyConcurrentBlockItem::HEIGHT + 2 * PADDING;
    auto it = mGroups.begin();
    for (size_t i = 0; i < tatami.groupCount(); ++i) {
        ConcurrentBlockItem *item = nullptr;
        if (it == mGroups.end() || (*it)->getHandle().id != tatami.getHandle(i).id) {
            // insert group
            item = new ConcurrentBlockItem(mStoreManager, mIndex, tatami.getHandle(i));
            mScene->addItem(item);
            mGroups.insert(it, item);

        }
        else {
            item = *it;
            if (affectedIds.find(item->getHandle().id) != affectedIds.end())
                item->reloadBlocks();

            std::advance(it, 1);
        }

        item->setPos(PADDING, offset);
        offset += item->getHeight() + PADDING;

        if (mEmptyGroups.size() < i + 2) {
            mEmptyGroups.push_back(new EmptyConcurrentBlockItem(mStoreManager, mIndex, i+1));
            mScene->addItem(mEmptyGroups.back());
        }

        mEmptyGroups[i+1]->setPos(PADDING, offset);
        offset += EmptyConcurrentBlockItem::HEIGHT + PADDING;
    }

    while (mEmptyGroups.size() > mGroups.size() + 1) {
        mScene->removeItem(mEmptyGroups.back());
        delete (mEmptyGroups.back());
        mEmptyGroups.pop_back();
    }

    // clear empty groups
}

void TatamiWidget::tatamisReset() {
    reloadBlocks();
}

void TatamiWidget::reloadBlocks() {
    const TournamentStore &tournament = mStoreManager->getTournament();
    const TatamiStore &tatami = tournament.getTatamis()[mIndex];

    mScene->clear();
    mEmptyGroups.clear();

    size_t offset = PADDING;

    {
        auto *emptyItem = new EmptyConcurrentBlockItem(mStoreManager, mIndex, 0);
        mEmptyGroups.push_back(emptyItem);
        mScene->addItem(emptyItem);

        emptyItem->setPos(PADDING, offset);
        offset += EmptyConcurrentBlockItem::HEIGHT + PADDING;
    }

    for (size_t i = 0; i < tatami.groupCount(); ++i) {
        auto *item = new ConcurrentBlockItem(mStoreManager, mIndex, tatami.getHandle(i));
        mScene->addItem(item);
        mGroups.push_back(item);

        item->setPos(PADDING, offset);
        offset += item->getHeight() + PADDING;

        auto *emptyItem = new EmptyConcurrentBlockItem(mStoreManager, mIndex, i+1);
        mEmptyGroups.push_back(emptyItem);
        mScene->addItem(emptyItem);

        emptyItem->setPos(PADDING, offset);
        offset += EmptyConcurrentBlockItem::HEIGHT + PADDING;
    }
}

EmptyConcurrentBlockItem::EmptyConcurrentBlockItem(StoreManager * storeManager, size_t tatamiIndex, size_t groupIndex)
    : mStoreManager(storeManager)
    , mTatamiIndex(tatamiIndex)
    , mGroupIndex(groupIndex)
    , mDragOver(false)
{
    setAcceptDrops(true);
}

QRectF EmptyConcurrentBlockItem::boundingRect() const {
    return QRectF(0, 0, WIDTH, HEIGHT);
}

void EmptyConcurrentBlockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);

    if (mDragOver)
        pen.setColor(COLOR_8);
    else
        pen.setColor(COLOR_4);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(0, 0, WIDTH, HEIGHT);
}

void EmptyConcurrentBlockItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    if (event->mimeData()->hasFormat("application/judoassistant-block")) {
        event->setAccepted(true);
        mDragOver = true;
    }
    else {
        event->setAccepted(false);
    }

    update();
}

void EmptyConcurrentBlockItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event) {
    mDragOver = false;
    update();
}

void EmptyConcurrentBlockItem::dropEvent(QGraphicsSceneDragDropEvent *event) {
    const auto * mime = dynamic_cast<const JudoassistantMime*>(event->mimeData());
    auto block = mime->block();
    QTournamentStore & tournament = mStoreManager->getTournament();
    TatamiStore & tatami = tournament.getTatamis()[mTatamiIndex];

    TatamiLocation location;
    location.tatamiIndex = mTatamiIndex;
    auto pair = tatami.addGroup(tournament, mGroupIndex);
    location.concurrentGroup = pair.first;
    location.sequentialGroup = pair.second;

    auto action = std::make_unique<SetTatamiLocationAction>(block.first, block.second, location, 0);
    mStoreManager->dispatch(std::move(action));
    mDragOver = false;
    update();
}

ConcurrentBlockItem::ConcurrentBlockItem(StoreManager * storeManager, size_t tatamiIndex, PositionHandle handle)
    : mStoreManager(storeManager)
    , mTatamiIndex(tatamiIndex)
    , mDragOver(false)
    , mHandle(handle)
{
    reloadBlocks();
    setAcceptDrops(true);
}

int ConcurrentBlockItem::getHeight() const {
    return mHeight;
}

QRectF ConcurrentBlockItem::boundingRect() const {
    return QRectF(0, 0, WIDTH, getHeight());
}

void ConcurrentBlockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);

    if (mDragOver)
        pen.setColor(COLOR_8);
    else
        pen.setColor(COLOR_4);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawRect(0, 0, WIDTH, getHeight());
}

void ConcurrentBlockItem::reloadBlocks() {
    for (auto * group : mSequentialGroups) {
        delete group;
    }

    mSequentialGroups.clear();

    auto & group = mStoreManager->getTournament().getTatamis()[mTatamiIndex].getGroup(mHandle);
    size_t offset = PADDING;
    int newHeight = 0;
    for (size_t i = 0; i < group.groupCount(); ++i) {
        auto * item = new SequentialBlockItem(mStoreManager, mTatamiIndex, mHandle, group.getHandle(i), this);
        item->setPos(offset, PADDING);
        offset += SequentialBlockItem::WIDTH + PADDING;
        newHeight = std::max(newHeight, item->getHeight());
        mSequentialGroups.push_back(item);
    }

    newHeight += PADDING*2;
    prepareGeometryChange();
    mHeight = newHeight;
    // update();
}

PositionHandle ConcurrentBlockItem::getHandle() const {
    return mHandle;
}

void ConcurrentBlockItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    if (event->mimeData()->hasFormat("application/judoassistant-block")) {
        event->setAccepted(true);
        mDragOver = true;
    }
    else {
        event->setAccepted(false);
    }

    update();
}

void ConcurrentBlockItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event) {
    mDragOver = false;
    update();
}

void ConcurrentBlockItem::dropEvent(QGraphicsSceneDragDropEvent *event) {
    const auto * mime = dynamic_cast<const JudoassistantMime*>(event->mimeData());
    auto block = mime->block();

    QTournamentStore & tournament = mStoreManager->getTournament();
    ConcurrentBlockGroup & group = tournament.getTatamis()[mTatamiIndex].getGroup(mHandle);

    TatamiLocation location;
    location.tatamiIndex = mTatamiIndex;
    location.concurrentGroup = mHandle;
    location.sequentialGroup = group.addGroup(tournament, mSequentialGroups.size());

    auto action = std::make_unique<SetTatamiLocationAction>(block.first, block.second, location, 0);
    mStoreManager->dispatch(std::move(action));
    mDragOver = false;
    update();
}

SequentialBlockItem::SequentialBlockItem(StoreManager * storeManager, size_t tatamiIndex, PositionHandle concurrentHandle, PositionHandle handle, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mStoreManager(storeManager)
    , mTatamiIndex(tatamiIndex)
    , mConcurrentHandle(concurrentHandle)
    , mHandle(handle)
    , mDragOver(false)
{
    setAcceptDrops(true);

    auto & group = mStoreManager->getTournament().getTatamis()[mTatamiIndex].getGroup(concurrentHandle).getGroup(handle);
    size_t offset = PADDING;
    for (size_t i = 0; i < group.blockCount(); ++i) {
        auto block = group.getBlock(i);
        auto *item = new BlockItem(mStoreManager, block.first, block.second, this);
        item->setPos(PADDING, offset);
        offset += item->getHeight() + BLOCK_MARGIN;
    }
    mHeight = offset + PADDING;
}

int SequentialBlockItem::getHeight() const {
    return mHeight;
}

QRectF SequentialBlockItem::boundingRect() const {
    return QRectF(0, 0, WIDTH, getHeight());
}

void SequentialBlockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);

    if (mDragOver)
        pen.setColor(COLOR_8);
    else
        pen.setColor(COLOR_4);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawRect(0, 0, WIDTH, getHeight());
}

void SequentialBlockItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    if (event->mimeData()->hasFormat("application/judoassistant-block")) {
        event->setAccepted(true);
        mDragOver = true;
    }
    else {
        event->setAccepted(false);
    }

    update();
}

void SequentialBlockItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event) {
    mDragOver = false;
    update();
}

void SequentialBlockItem::dropEvent(QGraphicsSceneDragDropEvent *event) {
    const auto * mime = dynamic_cast<const JudoassistantMime*>(event->mimeData());
    auto block = mime->block();

    TatamiLocation location;
    location.tatamiIndex = mTatamiIndex;
    location.concurrentGroup = mConcurrentHandle;
    location.sequentialGroup = mHandle;

    auto action = std::make_unique<SetTatamiLocationAction>(block.first, block.second, location, mBlocks.size());
    mStoreManager->dispatch(std::move(action));
    mDragOver = false;
    update();
}

BlockItem::BlockItem(StoreManager * storeManager, CategoryId categoryId, MatchType type, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mStoreManager(storeManager)
    , mType(type)
{
    mCategory = &mStoreManager->getTournament().getCategory(categoryId);
    mMatchCount = static_cast<int>(mCategory->getMatchCount(mType));
    setCursor(Qt::OpenHandCursor);
    setAcceptedMouseButtons(Qt::LeftButton);
}

int BlockItem::getHeight() const {
    return 30 + mMatchCount * 5;
}

QRectF BlockItem::boundingRect() const {
    return QRectF(0, 0, WIDTH, getHeight());
}

void BlockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(COLOR_3);

    painter->setPen(pen);
    painter->setBrush(COLOR_6);

    painter->drawRect(0, 0, WIDTH, getHeight());

    painter->setPen(COLOR_0);
    painter->setBrush(COLOR_0);
    painter->drawText(PADDING, PADDING+10, QString::fromStdString(mCategory->getName(mType)));

    QString countString = QString("(%1 matches)").arg(QString::number(mMatchCount)); // TODO: translate
    painter->drawText(PADDING, PADDING+25, countString);
}

void BlockItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::ClosedHandCursor);
}

void BlockItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    auto dist = QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length();
    if (dist < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(event->widget());
    JudoassistantMime *mime = new JudoassistantMime;
    drag->setMimeData(mime);

    // mime->setColorData(color);
    mime->setText(QString::fromStdString(mCategory->getName(mType)));
    mime->setBlock(mCategory->getId(), mType);

    QPixmap pixmap(WIDTH, getHeight());
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    // painter.translate(15, 15);
    painter.setRenderHint(QPainter::Antialiasing);
    paint(&painter, 0, 0);
    painter.end();

    pixmap.setMask(pixmap.createHeuristicMask());

    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(WIDTH/2, getHeight()/2));
    drag->exec();
}

void BlockItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::OpenHandCursor);
}
