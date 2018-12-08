#pragma once

#include <set>
#include <stack>
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
    TatamiMatchesModel(StoreManager &storeManager, size_t tatami, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::vector<MatchId> getMatches(const QItemSelection &selection) const;
    MatchId getMatch(int row) const;
    int getRow(MatchId id) const;

private slots:
    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
    void changeTatamis(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);

    void beginResetTournament();
    void endResetTournament();
private:
    const size_t COLUMN_COUNT = 1;
    const size_t ROW_CAP = 50; // The model will only load new blocks when it has less than ROW_CAP matches
    StoreManager & mStoreManager;
    size_t mTatami;
    size_t mBlocksLoaded;
    std::vector<MatchId> mIds;
    std::stack<QMetaObject::Connection> mConnections;
};

