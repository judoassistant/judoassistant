#pragma once

#include <QGraphicsItem>

#include "core/id.hpp"
#include "core/stores/match_store.hpp"

class StoreManager;
class TournamentStore;
class CategoryStore;

class MatchGraphicsItem : public QGraphicsItem {
public:
    static constexpr int WIDTH_HINT = 350;
    static constexpr int HEIGHT_HINT = 120;
    MatchGraphicsItem(const StoreManager &storeManager, CategoryId categoryId, MatchId matchId, QRect rect, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    void paintHeader(QPainter &painter, const QRect &rect, const CategoryStore &category, const MatchStore &match);
    void paintPlayer(QPainter &painter, const QRect &rect, MatchStore::PlayerIndex playerIndex, const TournamentStore &tournament, const CategoryStore &category, const MatchStore &match);

    const StoreManager &mStoreManager;
    CategoryId mCategoryId;
    MatchId mMatchId;
    QRect mRect;
};

