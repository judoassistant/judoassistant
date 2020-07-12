#include <algorithm>
#include <QApplication>
#include <QDragEnterEvent>
#include <QDragMoveEvent>

#include "core/actions/set_tatami_location_action.hpp"
#include "core/log.hpp"
#include "core/stores/category_store.hpp"
#include "ui/misc/judoassistant_mime.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/unallocated_blocks_widget.hpp"
#include "ui/widgets/graphics_items/unallocated_block_graphics_item.hpp"

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
    setMinimumSize(UnallocatedBlockGraphicsItem::WIDTH + PADDING*2 + 16, 800);
    setMaximumWidth(UnallocatedBlockGraphicsItem::WIDTH + PADDING*2 + 16);
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
        if (!category.getLocation(MatchType::ELIMINATION))
            shouldShift |= insertBlock(category, MatchType::ELIMINATION);
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
        if (!category.getLocation(MatchType::ELIMINATION))
            shouldShift |= insertBlock(category, MatchType::ELIMINATION);
        else
            shouldShift |= eraseBlock(category, MatchType::ELIMINATION);

        if (category.getDrawSystem().hasFinalBlock()) {
            if (!category.getLocation(MatchType::FINAL))
                shouldShift |= insertBlock(category, MatchType::FINAL);
            else
                shouldShift |= eraseBlock(category, MatchType::FINAL);
        }
        else {
            // If the final block was removed
            shouldShift |= eraseBlock(category, MatchType::FINAL);
        }
    }

    if (shouldShift)
        shiftBlocks();
}

void UnallocatedBlocksWidget::beginEraseCategories(std::vector<CategoryId> categoryIds) {
    bool shouldShift = false;
    for (auto categoryId : categoryIds) {
        shouldShift |= eraseBlock(categoryId, MatchType::ELIMINATION);
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
        if (!category.getLocation(MatchType::ELIMINATION))
            mBlocks.insert(std::make_pair(category.getId(), MatchType::ELIMINATION));
        if (category.getDrawSystem().hasFinalBlock() && !category.getLocation(MatchType::FINAL))
            mBlocks.insert(std::make_pair(category.getId(), MatchType::FINAL));
    }

    mBlockItems.clear();
    mScene->clear();

    size_t offset = PADDING;
    for (auto block : mBlocks) {
        const CategoryStore & category = tournament.getCategory(block.first);
        auto * item = new UnallocatedBlockGraphicsItem(category, block.second);
        item->setPos(PADDING, offset);
        mBlockItems[block] = item;
        offset += UnallocatedBlockGraphicsItem::HEIGHT + ITEM_MARGIN;
        mScene->addItem(item);
    }

    mScene->setSceneRect(0, 0, UnallocatedBlockGraphicsItem::WIDTH + PADDING*2, offset);
}

BlockComparator::BlockComparator(const TournamentStore &tournament) : mTournament(&tournament) {}

bool BlockComparator::operator()(const std::pair<CategoryId, MatchType> first, const std::pair<CategoryId, MatchType> second) const {
    const CategoryStore & firstCategory = mTournament->getCategory(first.first);
    const CategoryStore & secondCategory = mTournament->getCategory(second.first);

    auto firstName = firstCategory.getName(first.second);
    auto secondName = secondCategory.getName(second.second);

    if (firstName != secondName) // In case two categories have the same name
        return mComp(QString::fromStdString(firstName), QString::fromStdString(secondName));
    return first < second;
}

void UnallocatedBlocksWidget::shiftBlocks() {
    size_t offset = PADDING;
    for (auto block : mBlocks) {
        UnallocatedBlockGraphicsItem *item = mBlockItems[block];
        item->setPos(PADDING, offset);
        offset += UnallocatedBlockGraphicsItem::HEIGHT + ITEM_MARGIN;
    }

    mScene->setSceneRect(0, 0, UnallocatedBlockGraphicsItem::WIDTH + PADDING*2, offset);
}

bool UnallocatedBlocksWidget::insertBlock(const CategoryStore &category, MatchType type) {
    std::pair<CategoryId, MatchType> block = {category.getId(), type};

    auto res = mBlocks.insert(block);
    if (res.second) {
        auto * item = new UnallocatedBlockGraphicsItem(category, type);
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

void UnallocatedBlocksWidget::dragMoveEvent(QDragMoveEvent *event) {
    event->setAccepted(event->mimeData()->hasFormat("application/judoassistant-block"));
}

void UnallocatedBlocksWidget::dropEvent(QDropEvent *event) {
    const auto * mime = dynamic_cast<const JudoassistantMime*>(event->mimeData());
    auto block = mime->block();

    const auto &tournament = mStoreManager.getTournament();
    const auto &category = tournament.getCategory(block.first);

    if (!category.getLocation(block.second).has_value())
        return; // Unallocated block. No need to move

    mStoreManager.dispatch(std::make_unique<SetTatamiLocationAction>(block, std::nullopt));
}

