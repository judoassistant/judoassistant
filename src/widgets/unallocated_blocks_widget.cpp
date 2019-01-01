#include <algorithm>
#include <QGraphicsSceneDragDropEvent>
#include <QApplication>
#include <QDrag>
#include <QBitmap>
#include <QPen>

#include "stores/category_store.hpp"
#include "stores/qtournament_store.hpp"
#include "store_managers/store_manager.hpp"
#include "widgets/unallocated_blocks_widget.hpp"
#include "widgets/misc/judoassistant_mime.hpp"

UnallocatedBlocksWidget::UnallocatedBlocksWidget(StoreManager & storeManager, QWidget *parent)
    : QGraphicsView(parent)
    , mStoreManager(storeManager)
    , mBlocks(BlockComparator(storeManager.getTournament()))
{
    mScene = new QGraphicsScene(this);
    mScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    mScene->setSceneRect(0, 0, 200, 800);
    setScene(mScene);
    setCacheMode(CacheNone);
    setViewportUpdateMode(MinimalViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setMinimumSize(216, 400);
    setMaximumWidth(216);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &UnallocatedBlocksWidget::beginTournamentReset);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &UnallocatedBlocksWidget::endTournamentReset);
}

void UnallocatedBlocksWidget::changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {
    const TournamentStore & tournament = mStoreManager.getTournament();

    bool shouldShift = false;

    for (auto block : blocks) {
        if (!tournament.containsCategory(block.first))
            continue;

        const CategoryStore & category = tournament.getCategory(block.first);
        if (!category.getLocation(block.second))
            shouldShift |= insertBlock(category, block.second);
        else
            shouldShift |= eraseBlock(category, block.second);
    }

    if (shouldShift)
        shiftBlocks();
}

// TODO: Refactor the many nested loops for this class
void UnallocatedBlocksWidget::beginEraseTatamis(std::vector<TatamiLocation> locations) {
    bool shouldShift = false;

    TournamentStore &tournament = mStoreManager.getTournament();
    TatamiList &tatamis = tournament.getTatamis();

    for (TatamiLocation location : locations) {
        TatamiStore & tatami = tournament.getTatami(location);

        for (size_t i = 0; i < tatami.groupCount(); ++i) {
            auto &group = tatami.getGroup(i);

            for (size_t j = 0; j < group.groupCount(); ++j) {
                auto &seqGroup = group.getGroup(j);

                for (size_t k = 0; k < seqGroup.blockCount(); ++k) {
                    auto block = seqGroup.getBlock(k);
                    const CategoryStore & category = tournament.getCategory(block.first);
                    shouldShift |= insertBlock(category, block.second);
                }
            }
        }
    }

    if (shouldShift)
        shiftBlocks();
}

void UnallocatedBlocksWidget::endAddTatamis(std::vector<TatamiLocation> locations) {
    bool shouldShift = false;

    TournamentStore &tournament = mStoreManager.getTournament();
    TatamiList &tatamis = tournament.getTatamis();

    for (size_t index : indices) {
        for (size_t i = 0; i < tatami.groupCount(); ++i) {
            auto &group = tatami.getGroup(i);

            for (size_t j = 0; j < group.groupCount(); ++j) {
                auto &seqGroup = group.getGroup(j);

                for (size_t k = 0; k < seqGroup.blockCount(); ++k) {
                    auto block = seqGroup.getBlock(k);
                    shouldShift |= eraseBlock(block.first, block.second);
                }
            }
        }
    }

    if (shouldShift)
        shiftBlocks();
}

void UnallocatedBlocksWidget::endAddCategories(std::vector<CategoryId> categoryIds) {
    const TournamentStore & tournament = mStoreManager.getTournament();

    bool shouldShift = false;
    for (auto categoryId : categoryIds) {
        const CategoryStore & category = tournament.getCategory(categoryId);
        if (!category.getLocation(MatchType::KNOCKOUT))
            shouldShift |= insertBlock(category, MatchType::KNOCKOUT);
        if (category.getDrawSystem().hasFinalBlock() && !category.getLocation(MatchType::FINAL))
            shouldShift |= insertBlock(category, MatchType::FINAL);
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

    mConnections.push(connect(&tournament, &QTournamentStore::tatamisChanged, this, &UnallocatedBlocksWidget::changeTatamis));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeErased, this, &UnallocatedBlocksWidget::beginEraseTatamis));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAdded, this, &UnallocatedBlocksWidget::endAddTatamis));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesAdded, this, &UnallocatedBlocksWidget:endAddCategories));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesAboutToBeErased, this, &UnallocatedBlocksWidget::beginEraseCategories));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesReset, this, &UnallocatedBlocksWidget::endCategoriesReset));

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

    size_t offset = 0;
    for (auto block : mBlocks) {
        const CategoryStore & category = tournament.getCategory(block.first);
        auto * item = new UnallocatedBlockItem(category, block.second);
        item->setPos(0, offset);
        mBlockItems[block] = item;
        offset += UnallocatedBlockItem::HEIGHT + ITEM_MARGIN;
        mScene->addItem(item);
    }
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
    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(Qt::darkGray);

    painter->setPen(pen);
    painter->setBrush(Qt::lightGray);

    painter->drawRect(0, 0, WIDTH, HEIGHT);
    QString name = QString::fromStdString(mCategory->getName(mType));
    QString str = QString("%1 (%2 matches)").arg(name).arg(QString::number(mCategory->getMatchCount(mType))); // TODO: translate
    painter->drawText(PADDING, PADDING+10, str);
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
    painter.setRenderHint(QPainter::Antialiasing);
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
    size_t offset = 0;
    for (auto block : mBlocks) {
        UnallocatedBlockItem *item = mBlockItems[block];
        item->setPos(0, offset);
        offset += UnallocatedBlockItem::HEIGHT + ITEM_MARGIN;
    }
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

