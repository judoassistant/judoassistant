#include "core/stores/category_store.hpp"
#include "ui/models/match_card.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/graphics_items/match_graphics_item.hpp"

MatchGraphicsItem::MatchGraphicsItem(StoreManager &storeManager, const QPalette &palette, CategoryId categoryId, MatchId matchId, QRect rect, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mStoreManager(storeManager)
    , mPalette(palette)
    , mCategoryId(categoryId)
    , mMatchId(matchId)
    , mRect(rect)
{

}

QRectF MatchGraphicsItem::boundingRect() const {
    return mRect;
}

void MatchGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCategoryId))
        return;

    const auto &category = tournament.getCategory(mCategoryId);
    if (!category.containsMatch(mMatchId))
        return;

    const auto &match = category.getMatch(mMatchId);

    auto masterTime = mStoreManager.masterTime();
    MatchCard card(tournament, category, match, masterTime);
    card.paint(painter, mRect, mPalette);
}

