#pragma once

#include <QAbstractTableModel>
#include <QItemSelection>
#include <set>
#include <QSortFilterProxyModel>
#include "store_handlers/qstore_handler.hpp"

class PlayersModel : public QAbstractTableModel {
    Q_OBJECT
public:
    PlayersModel(QStoreHandler &storeHandler, QObject *parent);
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
    void playerCategoriesChanged(std::vector<PlayerId> playerIds);
    void categoriesAboutToBeErased(std::vector<CategoryId> playerIds);
    void categoriesErased(std::vector<CategoryId> playerIds);
private:
    std::string listPlayerCategories(const PlayerStore &player) const;
    const int COLUMN_COUNT = 9;
    QStoreHandler & mStoreHandler;
    std::set<PlayerId> mIds;
    std::unordered_set<PlayerId> mAffectedPlayers; // Used when receiving a categoriesAboutToBeErased signal
};

class PlayersProxyModel : public QSortFilterProxyModel {
public:
    PlayersProxyModel(QStoreHandler &storeHandler, QObject *parent);
    std::vector<PlayerId> getPlayers(const QItemSelection &selection) const;
    void setCategory(std::optional<CategoryId> categoryId);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void hideAll();

private:
    QStoreHandler & mStoreHandler;
    PlayersModel *mModel;
    std::optional<CategoryId> mCategoryId;
    bool mHidden;
};

