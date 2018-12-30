#pragma once

#include <stack>
#include <unordered_set>
#include <unordered_map>

#include <QMetaObject>
#include <QAbstractTableModel>
#include <QItemSelection>
#include <QSortFilterProxyModel>

#include "id.hpp"
#include "stores/category_store.hpp"
#include "stores/match_store.hpp"
#include "rulesets/ruleset.hpp"
#include "draw_systems/draw_system.hpp"

class StoreManager;

class CategoryMatchesModel : public QAbstractTableModel {
    Q_OBJECT
private:
    static const int COLUMN_COUNT = 1;
    static const int TIMER_INTERVAL = 1000;
public:
    CategoryMatchesModel(StoreManager &storeManager, QObject *parent);
    void setCategory(std::optional<CategoryId> categoryId);
    std::optional<CategoryId> getCategory() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::vector<MatchId> getMatches(const QItemSelection &selection) const;
    MatchId getMatch(int row) const;
    int getRow(MatchId matchId) const;

private:
    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
    void changePlayers(std::vector<PlayerId> playerIds);

    void beginResetTournament();
    void endResetTournament();

    void beginResetMatches();
    void endResetMatches();

    void timerHit();

    StoreManager & mStoreManager;
    std::optional<CategoryId> mCategoryId;

    std::vector<MatchId> mMatches;
    std::unordered_map<MatchId, size_t> mMatchesMap;
    std::unordered_map<PlayerId, std::unordered_set<MatchId>> mPlayers;
    std::unordered_map<MatchId, std::pair<std::optional<PlayerId>, std::optional<PlayerId>>> mMatchPlayerMap;
    std::unordered_set<MatchId> mUnpausedMatches;

    std::stack<QMetaObject::Connection> mConnections;
    bool mResettingMatches;
};

