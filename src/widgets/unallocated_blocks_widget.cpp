#include <QGraphicsSceneDragDropEvent>
#include <QApplication>
#include <QDrag>
#include <QBitmap>
#include <QPen>

#include "widgets/unallocated_blocks_widget.hpp"
#include <algorithm>
#include "widgets/misc/qutejudo_mime.hpp"

UnallocatedBlocksWidget::UnallocatedBlocksWidget(QStoreHandler & storeHandler, QWidget *parent)
    : QGraphicsView(parent)
    , mStoreHandler(storeHandler)
    , mBlocks(BlockComparator(storeHandler.getTournament()))
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

    auto & tournament = mStoreHandler.getTournament();

    connect(&tournament, &QTournamentStore::tatamisChanged, this, &UnallocatedBlocksWidget::tatamisChanged);
    connect(&tournament, &QTournamentStore::tatamisAboutToBeErased, this, &UnallocatedBlocksWidget::tatamisAboutToBeErased);
    connect(&tournament, &QTournamentStore::tatamisAdded, this, &UnallocatedBlocksWidget::tatamisAdded);
    connect(&tournament, &QTournamentStore::categoriesAdded, this, &UnallocatedBlocksWidget::categoriesAdded);
    connect(&tournament, &QTournamentStore::categoriesAboutToBeErased, this, &UnallocatedBlocksWidget::categoriesAboutToBeErased);
    connect(&tournament, &QTournamentStore::categoriesReset, this, &UnallocatedBlocksWidget::categoriesReset);
    connect(&mStoreHandler, &QStoreHandler::tournamentReset, this, &UnallocatedBlocksWidget::tournamentReset);

    reloadBlocks();
}

void UnallocatedBlocksWidget::tatamisChanged(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {
    const TournamentStore & tournament = mStoreHandler.getTournament();

    bool shouldShift = false;

    for (auto block : blocks) {
        const CategoryStore & category = tournament.getCategory(block.first);
        if (!category.getTatamiLocation(block.second))
            shouldShift |= insertBlock(category, block.second);
        else
            shouldShift |= eraseBlock(category, block.second);
    }

    if (shouldShift)
        shiftBlocks();
}

// TODO: Refactor the many nested loops for this class
void UnallocatedBlocksWidget::tatamisAboutToBeErased(std::vector<size_t> indices) {
    bool shouldShift = false;

    for (size_t index : indices) {
        TournamentStore & tournament = mStoreHandler.getTournament();
        TatamiStore & tatami = tournament.getTatamis()[index];

        for (size_t i = 0; i < tatami.groupCount(); ++i) {
            auto handle = tatami.getHandle(i);
            auto & group = tatami.getGroup(handle);

            for (size_t j = 0; j < group.groupCount(); ++j) {
                auto seqHandle = group.getHandle(j);
                auto & seqGroup = group.getGroup(seqHandle);

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

void UnallocatedBlocksWidget::tatamisAdded(std::vector<size_t> indices) {
    bool shouldShift = false;

    for (size_t index : indices) {
        TatamiStore & tatami = mStoreHandler.getTournament().getTatamis()[index];

        for (size_t i = 0; i < tatami.groupCount(); ++i) {
            auto handle = tatami.getHandle(i);
            auto & group = tatami.getGroup(handle);

            for (size_t j = 0; j < group.groupCount(); ++j) {
                auto seqHandle = group.getHandle(j);
                auto & seqGroup = group.getGroup(seqHandle);

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

void UnallocatedBlocksWidget::categoriesAdded(std::vector<CategoryId> categoryIds) {
    const TournamentStore & tournament = mStoreHandler.getTournament();

    bool shouldShift = false;
    for (auto categoryId : categoryIds) {
        const CategoryStore & category = tournament.getCategory(categoryId);
        if (!category.getTatamiLocation(MatchType::KNOCKOUT))
            shouldShift |= insertBlock(category, MatchType::KNOCKOUT);
        if (category.getDrawSystem().hasFinalBlock() && !category.getTatamiLocation(MatchType::FINAL))
            shouldShift |= insertBlock(category, MatchType::FINAL);
    }

    if (shouldShift)
        shiftBlocks();
}

void UnallocatedBlocksWidget::categoriesAboutToBeErased(std::vector<CategoryId> categoryIds) {
    bool shouldShift = false;
    for (auto categoryId : categoryIds) {
        shouldShift |= eraseBlock(categoryId, MatchType::KNOCKOUT);
        shouldShift |= eraseBlock(categoryId, MatchType::FINAL);
    }

    if (shouldShift)
        shiftBlocks();
}

void UnallocatedBlocksWidget::tournamentReset() {
    auto & tournament = mStoreHandler.getTournament();

    connect(&tournament, &QTournamentStore::tatamisChanged, this, &UnallocatedBlocksWidget::tatamisChanged);
    connect(&tournament, &QTournamentStore::tatamisAboutToBeErased, this, &UnallocatedBlocksWidget::tatamisAboutToBeErased);
    connect(&tournament, &QTournamentStore::tatamisAdded, this, &UnallocatedBlocksWidget::tatamisAdded);
    connect(&tournament, &QTournamentStore::categoriesAdded, this, &UnallocatedBlocksWidget::categoriesAdded);
    connect(&tournament, &QTournamentStore::categoriesAboutToBeErased, this, &UnallocatedBlocksWidget::categoriesAboutToBeErased);
    connect(&tournament, &QTournamentStore::categoriesReset, this, &UnallocatedBlocksWidget::categoriesReset);

    mBlocks = std::set<std::pair<CategoryId, MatchType>, BlockComparator>(BlockComparator(tournament));
    reloadBlocks();
}

void UnallocatedBlocksWidget::categoriesReset() {
    reloadBlocks();
}

void UnallocatedBlocksWidget::reloadBlocks() {
    mBlocks.clear();
    const TournamentStore & tournament = mStoreHandler.getTournament();

    for (auto & it : tournament.getCategories()) {
        const CategoryStore & category = *(it.second);
        if (!category.getTatamiLocation(MatchType::KNOCKOUT))
            mBlocks.insert(std::make_pair(category.getId(), MatchType::KNOCKOUT));
        if (category.getDrawSystem().hasFinalBlock() && !category.getTatamiLocation(MatchType::FINAL))
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
        return firstName < secondName;
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
    painter->drawText(PADDING, PADDING+10, QString::fromStdString(mCategory->getName(mType)));
}

void UnallocatedBlockItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::ClosedHandCursor);
}

void UnallocatedBlockItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    auto dist = QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length();
    if (dist < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(event->widget());
    QutejudoMime *mime = new QutejudoMime;
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
