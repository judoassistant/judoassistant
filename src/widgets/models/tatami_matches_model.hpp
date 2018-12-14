#pragma once

#include <set>
#include <stack>
#include <deque>
#include <unordered_set>

#include <QMetaObject>
#include <QAbstractTableModel>
#include <QItemSelection>
#include <QSortFilterProxyModel>

#include "id.hpp"
#include "stores/tatami_store.hpp"
#include "stores/match_store.hpp"
#include "stores/match_event.hpp"

class StoreManager;

class TatamiMatchesModel : public QAbstractTableModel {
    Q_OBJECT
public:
    TatamiMatchesModel(StoreManager &storeManager, size_t tatami, size_t rowCap, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::vector<MatchId> getMatches(const QItemSelection &selection) const;
    MatchId getMatch(int row) const;
    int getRow(MatchId id) const;

private:
    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
    void changeTatamis(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);

    void beginResetTournament();
    void endResetTournament();

    void beginResetMatches();
    void endResetMatches();

    void loadBlocks(bool shouldSignal = true);

    void beginResetCategory(CategoryId categoryId);

    const size_t COLUMN_COUNT = 1;

    StoreManager & mStoreManager;
    size_t mTatami;
    size_t mRowCap;
    bool mResettingMatches;
    std::unordered_map<MatchId, size_t> mLoadedMatches; // Matches loaded and loading time
    std::unordered_set<PositionId> mLoadedBlocks; // Blocks loaded

    std::deque<std::pair<MatchId, size_t>> mUnfinishedMatches; // Unfinished (and loaded) matches and loading time
    std::unordered_set<MatchId> mUnfinishedMatchesSet;

    std::stack<QMetaObject::Connection> mConnections;

    // TODO: Find an implementation of a RB-tree augmented with size
    // TODO: Connect to begin reset matches
};

