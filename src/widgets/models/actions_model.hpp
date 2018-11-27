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

class ActionsModel : public QAbstractTableModel {
    Q_OBJECT
public:
    ActionsModel(StoreManager &storeManager, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::vector<ClientActionId> getActions(const QItemSelection &selection) const;
    ClientActionId getAction(int row) const;
    int getRow(ClientActionId id) const;
public slots:
    void addAction(ClientActionId actionId, size_t pos);
    void eraseAction(ClientActionId actionId);
    void aboutToReset();
    void reset();
private:
    const int COLUMN_COUNT = 2;
    StoreManager & mStoreManager;
    std::vector<ClientActionId> mIds; // TODO: Consider using std::list and map instead
};

class ActionsProxyModel : public QSortFilterProxyModel {
public:
    ActionsProxyModel(StoreManager &storeManager, QObject *parent);
    std::vector<ClientActionId> getActions(const QItemSelection &selection) const;
    void setClientId(std::optional<ClientId> clientId);
    // void setMatchId(std::optional<MatchId> clientId);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void hideAll();

private:
    StoreManager & mStoreManager;
    ActionsModel *mModel;
    std::optional<ClientId> mClientId;
    bool mHidden;
};

