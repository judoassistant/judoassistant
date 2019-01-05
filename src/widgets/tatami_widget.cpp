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

TatamiWidget::TatamiWidget(StoreManager & storeManager, TatamiLocation location, QWidget *parent)
    : QGraphicsView(parent)
    , mStoreManager(&storeManager)
    , mLocation(location)
{
    QTournamentStore & tournament = mStoreManager->getTournament();

    connect(&tournament, &QTournamentStore::tatamisChanged, this, &TatamiWidget::changeTatamis);
    connect(&tournament, &QTournamentStore::categoriesReset, this, &TatamiWidget::endResetTatamis);

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

void TatamiWidget::changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {
    std::unordered_set<PositionId> affectedConcurrentGroups;
    for (auto location : locations) {
        if (!location.sequentialGroup.concurrentGroup.tatami.equiv(mLocation)) continue;

        affectedConcurrentGroups.insert(location.sequentialGroup.concurrentGroup.handle.id);
    }

    if (affectedConcurrentGroups.empty())
        return;

    const auto &tatamis = mStoreManager->getTournament().getTatamis();
    const auto &tatami = tatamis.at(mLocation);

    for (auto i = mGroups.begin(); i != mGroups.end();) {
        auto next = std::next(i);
        PositionHandle handle = (*i)->getLocation().handle;
        if (!tatami.containsGroup(handle)) {
            mScene->removeItem(*i);
            delete (*i);
            mGroups.erase(i);
        }

        i = next;
    }

    // There is always an empty block at the top
    size_t offset = EmptyConcurrentBlockItem::HEIGHT + 2 * PADDING;
    auto it = mGroups.begin();
    for (size_t i = 0; i < tatami.groupCount(); ++i) {
        ConcurrentBlockItem *item = nullptr;
        PositionHandle handle = tatami.getHandle(i);
        if (it == mGroups.end() || !(handle.equiv((*it)->getLocation().handle))) {
            // insert group
            item = new ConcurrentBlockItem(mStoreManager, {mLocation, handle});
            mScene->addItem(item);
            mGroups.insert(it, item);

        }
        else {
            item = *it;
            if (affectedConcurrentGroups.find(item->getLocation().handle.id) != affectedConcurrentGroups.end())
                item->reloadBlocks();

            std::advance(it, 1);
        }

        item->setPos(PADDING, offset);
        offset += item->getHeight() + PADDING;

        if (mEmptyGroups.size() < i + 2) {
            mEmptyGroups.push_back(new EmptyConcurrentBlockItem(mStoreManager, mLocation, i+1));
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

void TatamiWidget::endResetTatamis() {
    reloadBlocks();
}

void TatamiWidget::reloadBlocks() {
    const TournamentStore &tournament = mStoreManager->getTournament();
    const TatamiStore &tatami = tournament.getTatamis().at(mLocation);

    mScene->clear();
    mEmptyGroups.clear();

    size_t offset = PADDING;

    {
        auto *emptyItem = new EmptyConcurrentBlockItem(mStoreManager, mLocation, 0);
        mEmptyGroups.push_back(emptyItem);
        mScene->addItem(emptyItem);

        emptyItem->setPos(PADDING, offset);
        offset += EmptyConcurrentBlockItem::HEIGHT + PADDING;
    }

    for (size_t i = 0; i < tatami.groupCount(); ++i) {
        auto *item = new ConcurrentBlockItem(mStoreManager, {mLocation, tatami.getHandle(i)});
        mScene->addItem(item);
        mGroups.push_back(item);

        item->setPos(PADDING, offset);
        offset += item->getHeight() + PADDING;

        auto *emptyItem = new EmptyConcurrentBlockItem(mStoreManager, mLocation, i+1);
        mEmptyGroups.push_back(emptyItem);
        mScene->addItem(emptyItem);

        emptyItem->setPos(PADDING, offset);
        offset += EmptyConcurrentBlockItem::HEIGHT + PADDING;
    }
}

EmptyConcurrentBlockItem::EmptyConcurrentBlockItem(StoreManager * storeManager, TatamiLocation tatami, size_t index)
    : mStoreManager(storeManager)
    , mTatami(tatami)
    , mIndex(index)
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

ConcurrentBlockItem::ConcurrentBlockItem(StoreManager * storeManager, ConcurrentGroupLocation location)
    : mStoreManager(storeManager)
    , mLocation(location)
    , mDragOver(false)
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

    const auto &tatamis = mStoreManager->getTournament().getTatamis();
    const auto &group = tatamis.at(mLocation);

    size_t offset = PADDING;
    int newHeight = 0;
    for (size_t i = 0; i < group.groupCount(); ++i) {
        auto * item = new SequentialBlockItem(mStoreManager, {mLocation, group.getHandle(i)}, this);
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

ConcurrentGroupLocation  ConcurrentBlockItem::getLocation() const {
    return mLocation;
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

    auto &tournament = mStoreManager->getTournament();
    auto &group = tournament.getTatamis().at(mLocation);

    BlockLocation location;
    location.sequentialGroup = group.generateLocation(mLocation, mSequentialGroups.size());
    location.pos = 0;

    mStoreManager->dispatch(std::make_unique<SetTatamiLocationAction>(block, location));
    mDragOver = false;
    update();
}

SequentialBlockItem::SequentialBlockItem(StoreManager * storeManager, SequentialGroupLocation location, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mStoreManager(storeManager)
    , mLocation(location)
    , mDragOver(false)
{
    setAcceptDrops(true);

    const auto &tatamis = mStoreManager->getTournament().getTatamis();
    const auto &group = tatamis.at(mLocation);
    size_t offset = PADDING;
    for (size_t i = 0; i < group.blockCount(); ++i) {
        auto block = group.at(i);
        auto *item = new BlockItem(mStoreManager, block, this);
        item->setPos(PADDING, offset);
        offset += item->getHeight() + BLOCK_MARGIN;
        mBlocks.push_back(block);
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

    BlockLocation location;
    location.sequentialGroup = mLocation;
    location.pos = mBlocks.size();

    mStoreManager->dispatch(std::make_unique<SetTatamiLocationAction>(block, location));
    mDragOver = false;
    update();
}

BlockItem::BlockItem(StoreManager * storeManager, std::pair<CategoryId, MatchType> block, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mStoreManager(storeManager)
    , mBlock(block)
{
    const auto &category = &mStoreManager->getTournament().getCategory(block.first);
    mName = QString::fromStdString(category->getName(block.second));
    mMatchCount = static_cast<int>(category->getMatchCount(block.second));

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
    painter->drawText(PADDING, PADDING+10, mName);

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
    mime->setText(mName);
    mime->setBlock(mBlock.first, mBlock.second);

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

