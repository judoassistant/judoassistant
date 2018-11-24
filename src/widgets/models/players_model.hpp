#pragma once

#include <set>
#include <stack>
#include <unordered_set>
#include <QMetaObject>
#include <QAbstractTableModel>
#include <QItemSelection>
#include <QSortFilterProxyModel>

#include "id.hpp"

class StoreManager;
class PlayerStore;

class PlayersModel : public QAbstractTableModel {
    Q_OBJECT
public:
    PlayersModel(StoreManager &storeManager, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::vector<PlayerId> getPlayers(const QItemSelection &selection) const;
    PlayerId getPlayer(int row) const;

    int getRow(PlayerId id) const;
    std::vector<int> getRows(std::vector<PlayerId> id) const;
public slots:
    void playersAdded(std::vector<PlayerId> ids);
    void playersChanged(std::vector<PlayerId> ids);
    void playersAboutToBeErased(std::vector<PlayerId> ids);
    void playersAboutToBeReset();
    void playersReset();
    void tournamentReset();
    void tournamentAboutToBeReset();
    void playerCategoriesChanged(CategoryId categoryId, std::vector<PlayerId> playerIds);
    void playerCategoriesChanged(std::vector<PlayerId> playerIds);
    void categoriesAboutToBeErased(std::vector<CategoryId> playerIds);
    void categoriesErased(std::vector<CategoryId> playerIds);
private:
    std::string listPlayerCategories(const PlayerStore &player) const;
    const int COLUMN_COUNT = 9;
    StoreManager & mStoreManager;
    std::set<PlayerId> mIds;
    std::unordered_set<PlayerId> mAffectedPlayers; // Used when receiving a categoriesAboutToBeErased signal
    std::stack<QMetaObject::Connection> mConnections;
};

class PlayersProxyModel : public QSortFilterProxyModel {
public:
    PlayersProxyModel(StoreManager &storeManager, QObject *parent);
    std::vector<PlayerId> getPlayers(const QItemSelection &selection) const;
    void setCategory(std::optional<CategoryId> categoryId);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void hideAll();

private:
    StoreManager & mStoreManager;
    PlayersModel *mModel;
    std::optional<CategoryId> mCategoryId;
    bool mHidden;
};

