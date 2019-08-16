#pragma once

#include <set>
#include <map>
#include <stack>

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QMetaObject>

#include "core/id.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/tatami/location.hpp"
#include "ui/misc/numerical_string_comparator.hpp"

class StoreManager;
class CategoryStore;
class UnallocatedBlockGraphicsItem;

class BlockComparator {
public:
    BlockComparator(const BlockComparator &other) = default;
    BlockComparator(const TournamentStore &tournament);
    bool operator()(const std::pair<CategoryId, MatchType>, const std::pair<CategoryId, MatchType>) const;
private:
    const TournamentStore *mTournament;
    NumericalStringComparator mComp;
};

class UnallocatedBlocksWidget : public QGraphicsView {
    Q_OBJECT
public:
    static const int ITEM_MARGIN = 3;
    static const int PADDING = 5;

    UnallocatedBlocksWidget(StoreManager & storeManager, QWidget *parent = 0);

    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

protected slots:
    void endAddCategories(std::vector<CategoryId> categoryIds);
    void beginEraseCategories(std::vector<CategoryId> categoryIds);
    void changeCategories(std::vector<CategoryId> categoryIds);
    void beginTournamentReset();
    void endTournamentReset();
    void endCategoriesReset();

private:
    void reloadBlocks();
    void shiftBlocks();
    bool insertBlock(const CategoryStore &category, MatchType type);
    bool eraseBlock(CategoryId id, MatchType type);
    bool eraseBlock(const CategoryStore &category, MatchType type);

    QGraphicsScene *mScene;
    StoreManager & mStoreManager;
    std::set<std::pair<CategoryId, MatchType>, BlockComparator> mBlocks;
    std::map<std::pair<CategoryId, MatchType>, UnallocatedBlockGraphicsItem*> mBlockItems;
    std::stack<QMetaObject::Connection> mConnections;
};

