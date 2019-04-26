#pragma once

#include <stack>
#include <unordered_map>
#include <QMetaObject>
#include <QAbstractTableModel>

#include "core/id.hpp"

enum class MatchStatus;
class StoreManager;
class PlayerStore;

class ResultsModel : public QAbstractTableModel {
    Q_OBJECT
public:
    ResultsModel(StoreManager &storeManager, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // std::vector<PlayerId> getPlayers(const QItemSelection &selection) const;
    // PlayerId getPlayer(int row) const;

    // int getRow(PlayerId id) const;
    // std::vector<int> getRows(std::vector<PlayerId> id) const;

    void setCategory(std::optional<CategoryId> categoryId);
    std::optional<CategoryId> getCategory() const;

private:
    void changePlayers(const std::vector<PlayerId> &playerIds);
    void beginErasePlayers(const std::vector<PlayerId> &playerIds);
    void beginResetPlayers();

    void beginResetTournament();
    void endResetTournament();

    void resetCategoryResults(CategoryId categoryId);

    void beginResetResults();
    void endResetResults();
    const int COLUMN_COUNT = 3;

    StoreManager & mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;
    std::optional<CategoryId> mCategory;

    bool mResetting;
    std::unordered_map<PlayerId, size_t> mPlayers;
    std::vector<std::pair<PlayerId, std::optional<unsigned int>>> mResults;
};

