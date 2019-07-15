#pragma once

#include <optional>
#include <set>
#include <stack>
#include <unordered_set>
#include <QMetaObject>
#include <QAbstractTableModel>
#include <QItemSelection>
#include <QSortFilterProxyModel>

#include "core/id.hpp"

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

    int getRow(PlayerId playerId) const;

protected:
    void playersAdded(const std::vector<PlayerId> &playerIds);
    void playersChanged(const std::vector<PlayerId> &playerIds);
    void playersAboutToBeErased(const std::vector<PlayerId> &playerIds);
    void playersAboutToBeReset();
    void playersReset();
    void tournamentReset();
    void tournamentAboutToBeReset();
    void playerCategoriesChanged(CategoryId categoryId, const std::vector<PlayerId> &playerIds);
    void playerCategoriesChanged(const std::vector<PlayerId> &playerIds);
    void categoriesAboutToBeErased(const std::vector<CategoryId> &categoryIds);
    void categoriesErased(const std::vector<CategoryId> &categoryIds);

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
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    std::optional<CategoryId> getCategory() const;
    void showU12(bool checked);
    void showU15(bool checked);
    void showU18(bool checked);
    void showU21(bool checked);
    void showSenior(bool checked);
    void showMale(bool checked);
    void showFemale(bool checked);

private:
    StoreManager & mStoreManager;
    PlayersModel *mModel;
    std::optional<CategoryId> mCategoryId;
    bool mHidden;
    bool mShowU12;
    bool mShowU15;
    bool mShowU18;
    bool mShowU21;
    bool mShowSenior;
    bool mShowMale;
    bool mShowFemale;
};

