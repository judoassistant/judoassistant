#include <algorithm>
#include <QGraphicsSceneDragDropEvent>
#include <QApplication>
#include <QDrag>
#include <QBitmap>
#include <QPen>

#include "core/stores/category_store.hpp"
#include "ui/misc/judoassistant_mime.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/unallocated_blocks_widget.hpp"

UnallocatedBlocksWidget::UnallocatedBlocksWidget(StoreManager & storeManager, QWidget *parent)
    : QGraphicsView(parent)
    , mStoreManager(storeManager)
    , mBlocks(BlockComparator(storeManager.getTournament()))
{
    mScene = new QGraphicsScene(this);
    mScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    setScene(mScene);
    setCacheMode(CacheNone);
    setViewportUpdateMode(MinimalViewportUpdate);
    setRenderHint(QPainter::Antialiasing, false);
    setMinimumSize(UnallocatedBlockItem::WIDTH + PADDING*2 + 16, 800);
    setMaximumWidth(UnallocatedBlockItem::WIDTH + PADDING*2 + 16);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &UnallocatedBlocksWidget::beginTournamentReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &UnallocatedBlocksWidget::endTournamentReset);

    reloadBlocks();
}

void UnallocatedBlocksWidget::endAddCategories(std::vector<CategoryId> categoryIds) {
    const TournamentStore & tournament = mStoreManager.getTournament();

    bool shouldShift = false;
    for (auto categoryId : categoryIds) {
        const CategoryStore &category = tournament.getCategory(categoryId);
        if (!category.getLocation(MatchType::KNOCKOUT))
            shouldShift |= insertBlock(category, MatchType::KNOCKOUT);
        if (category.getDrawSystem().hasFinalBlock() && !category.getLocation(MatchType::FINAL))
            shouldShift |= insertBlock(category, MatchType::FINAL);
    }

    if (shouldShift)
        shiftBlocks();
}

void UnallocatedBlocksWidget::changeCategories(std::vector<CategoryId> categoryIds) {
    const TournamentStore & tournament = mStoreManager.getTournament();

    bool shouldShift = false;
    for (auto categoryId : categoryIds) {
        const CategoryStore &category = tournament.getCategory(categoryId);
        if (!category.getLocation(MatchType::KNOCKOUT))
            shouldShift |= insertBlock(category, MatchType::KNOCKOUT);
        else
            shouldShift |= eraseBlock(category, MatchType::KNOCKOUT);

        if (category.getDrawSystem().hasFinalBlock()) {
            if (!category.getLocation(MatchType::FINAL))
                shouldShift |= insertBlock(category, MatchType::FINAL);
            else
                shouldShift |= eraseBlock(category, MatchType::FINAL);
        }
    }

    if (shouldShift)
        shiftBlocks();
}

void UnallocatedBlocksWidget::beginEraseCategories(std::vector<CategoryId> categoryIds) {
    bool shouldShift = false;
    for (auto categoryId : categoryIds) {
        shouldShift |= eraseBlock(categoryId, MatchType::KNOCKOUT);
        shouldShift |= eraseBlock(categoryId, MatchType::FINAL);
    }

    if (shouldShift)
        shiftBlocks();
}

void UnallocatedBlocksWidget::beginTournamentReset() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }

    mBlocks.clear();
}

void UnallocatedBlocksWidget::endTournamentReset() {
    auto & tournament = mStoreManager.getTournament();

    mConnections.push(connect(&tournament, &QTournamentStore::categoriesAdded, this, &UnallocatedBlocksWidget::endAddCategories));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesAboutToBeErased, this, &UnallocatedBlocksWidget::beginEraseCategories));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesReset, this, &UnallocatedBlocksWidget::endCategoriesReset));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesChanged, this, &UnallocatedBlocksWidget::changeCategories));

    mBlocks = std::set<std::pair<CategoryId, MatchType>, BlockComparator>(BlockComparator(tournament));
    reloadBlocks();
}

void UnallocatedBlocksWidget::endCategoriesReset() {
    reloadBlocks();
}

void UnallocatedBlocksWidget::reloadBlocks() {
    mBlocks.clear();
    const TournamentStore & tournament = mStoreManager.getTournament();

    for (auto & it : tournament.getCategories()) {
        const CategoryStore & category = *(it.second);
        if (!category.getLocation(MatchType::KNOCKOUT))
            mBlocks.insert(std::make_pair(category.getId(), MatchType::KNOCKOUT));
        if (category.getDrawSystem().hasFinalBlock() && !category.getLocation(MatchType::FINAL))
            mBlocks.insert(std::make_pair(category.getId(), MatchType::FINAL));
    }

    mBlockItems.clear();
    mScene->clear();

    size_t offset = PADDING;
    for (auto block : mBlocks) {
        const CategoryStore & category = tournament.getCategory(block.first);
        auto * item = new UnallocatedBlockItem(category, block.second);
        item->setPos(PADDING, offset);
        mBlockItems[block] = item;
        offset += UnallocatedBlockItem::HEIGHT + ITEM_MARGIN;
        mScene->addItem(item);
    }

    mScene->setSceneRect(0, 0, UnallocatedBlockItem::WIDTH + PADDING*2, offset);
}

BlockComparator::BlockComparator(const TournamentStore &tournament) : mTournament(&tournament) {}

bool BlockComparator::operator()(const std::pair<CategoryId, MatchType> first, const std::pair<CategoryId, MatchType> second) const {
    const CategoryStore & firstCategory = mTournament->getCategory(first.first);
    const CategoryStore & secondCategory = mTournament->getCategory(second.first);

    auto firstName = firstCategory.getName(first.second);
    auto secondName = secondCategory.getName(second.second);

    if (firstName != secondName) // In case two categories have the same name
        return mComp(firstName, secondName);
    return first < second;
}

UnallocatedBlockItem::UnallocatedBlockItem(const CategoryStore &category, MatchType type)
    : mCategory(&category)
    , mType(type)
{
    setCursor(Qt::OpenHandCursor);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF UnallocatedBlockItem::boundingRect() const {
    return QRectF(0, 0, WIDTH, HEIGHT);
}

void UnallocatedBlockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(COLOR_14);

    const auto &categoryStatus = mCategory->getStatus(mType);
    if (categoryStatus.startedMatches == 0 && categoryStatus.finishedMatches == 0)
        painter->setBrush(COLOR_14);
    else if (categoryStatus.startedMatches > 0 || categoryStatus.notStartedMatches > 0)
        painter->setBrush(COLOR_13);
    else
        painter->setBrush(COLOR_11);

    QRect rect(0, 0, WIDTH, HEIGHT);
    painter->drawRect(rect);

    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(COLOR_0);
    painter->setPen(pen);

    QRect titleRect(PADDING, PADDING, WIDTH-PADDING*2, 20);
    QRect typeRect(PADDING*5, 20+PADDING, WIDTH-PADDING*6, 20);
    QRect timeRect(PADDING*5, 40+PADDING, WIDTH-PADDING*6, 20);

    QString title = QString::fromStdString(mCategory->getName());
    painter->drawText(titleRect, Qt::AlignTop | Qt::AlignLeft, title);

    QString type = (mType == MatchType::FINAL ? QObject::tr("Finals") : QObject::tr("Elimination"));
    painter->drawText(typeRect, Qt::AlignTop | Qt::AlignLeft, type);

    unsigned int minutes = std::chrono::duration_cast<std::chrono::minutes>(mCategory->getRuleset().getEstimatedTime()).count() * mCategory->getMatchCount(mType);
    QString time = QObject::tr("~ %1 min").arg(minutes);
    painter->drawText(timeRect, Qt::AlignTop | Qt::AlignLeft, time);
}

void UnallocatedBlockItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::ClosedHandCursor);
}

void UnallocatedBlockItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
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

void UnallocatedBlockItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::OpenHandCursor);
}

void UnallocatedBlocksWidget::shiftBlocks() {
    size_t offset = PADDING;
    for (auto block : mBlocks) {
        UnallocatedBlockItem *item = mBlockItems[block];
        item->setPos(PADDING, offset);
        offset += UnallocatedBlockItem::HEIGHT + ITEM_MARGIN;
    }

    mScene->setSceneRect(0, 0, UnallocatedBlockItem::WIDTH + PADDING*2, offset);
}

bool UnallocatedBlocksWidget::insertBlock(const CategoryStore &category, MatchType type) {
    std::pair<CategoryId, MatchType> block = {category.getId(), type};

    auto res = mBlocks.insert(block);
    if (res.second) {
        auto * item = new UnallocatedBlockItem(category, type);
        mBlockItems[block] = item;
        mScene->addItem(item);
    }

    return res.second;
}

bool UnallocatedBlocksWidget::eraseBlock(CategoryId id, MatchType type) {
    std::pair<CategoryId, MatchType> block = {id, type};

    if (mBlocks.erase(block) > 0) {
        auto it = mBlockItems.find(block);

        mScene->removeItem(it->second);
        delete (it->second);
        mBlockItems.erase(it);

        return true;
    }

    return false;
}

bool UnallocatedBlocksWidget::eraseBlock(const CategoryStore &category, MatchType type) {
    return eraseBlock(category.getId(), type);
}

