#pragma once

#include <set>
#include <stack>
#include <deque>
#include <unordered_set>
#include <unordered_map>

#include <QMetaObject>
#include <QAbstractTableModel>
#include <QItemSelection>
#include <QSortFilterProxyModel>

#include "core/id.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/tatami/location.hpp"

class StoreManager;

class TatamiMatchesModel : public QAbstractTableModel {
    Q_OBJECT
private:
    static const int COLUMN_COUNT = 1;
    static constexpr auto TIMER_INTERVAL = std::chrono::milliseconds(1000);
public:
    TatamiMatchesModel(StoreManager &storeManager, TatamiLocation tatami, size_t rowCap, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::vector<std::pair<CategoryId, MatchId>> getMatches(const QItemSelection &selection) const;
    std::pair<CategoryId, MatchId> getMatch(int row) const;
    int getRow(CategoryId categoryId, MatchId matchId) const;
    int getRow(std::pair<CategoryId, MatchId> combinedId) const;

protected:
    void changeMatches(CategoryId categoryId, const std::vector<MatchId> &matchIds);
    void changeTatamis(const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks);
    void changePlayers(const std::vector<PlayerId> &playerIds);
    void timerHit();

    void beginResetTournament();
    void endResetTournament();

    void beginResetMatches();
    void endResetMatches();

    void loadBlocks(bool shouldSignal = true);

    void beginResetCategory(CategoryId categoryId);

private:
    StoreManager & mStoreManager;
    TatamiLocation mTatami;
    size_t mRowCap;
    bool mResettingMatches;
    std::unordered_map<std::pair<CategoryId,MatchId>, size_t> mLoadedMatches; // Matches loaded and loading time
    std::unordered_set<PositionId> mLoadedGroups; // Blocks loaded

    std::deque<std::tuple<CategoryId, MatchId, size_t>> mUnfinishedMatches; // Unfinished (and loaded) matches and loading time
    std::unordered_set<std::pair<CategoryId, MatchId>> mUnfinishedMatchesSet;

    std::unordered_map<PlayerId, std::unordered_set<std::pair<CategoryId, MatchId>>> mUnfinishedMatchesPlayers;
    std::unordered_map<std::pair<CategoryId, MatchId>, std::pair<std::optional<PlayerId>, std::optional<PlayerId>>> mUnfinishedMatchesPlayersInv;
    std::unordered_set<std::pair<CategoryId, MatchId>> mUnpausedMatches;

    std::stack<QMetaObject::Connection> mConnections;
};

