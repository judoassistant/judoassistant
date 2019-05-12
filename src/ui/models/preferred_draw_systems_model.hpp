#pragma once

#include <stack>

#include <QAbstractTableModel>
#include <QMetaObject>

#include "ui/stores/qtournament_store.hpp"
#include "core/stores/preferences_store.hpp"

class StoreManager;

class PreferredDrawSystemsModel : public QAbstractTableModel {
    Q_OBJECT
public:
    PreferredDrawSystemsModel(StoreManager &storeManager, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    const int COLUMN_COUNT = 2;

    void beginResetTournament();
    void endResetTournament();

    void changePreferences();

    StoreManager &mStoreManager;
    std::stack<QMetaObject::Connection> mConnections;
    std::vector<DrawSystemPreference> mPreferredDrawSystems;
};

