#pragma once
#include <QAbstractTableModel>
#include <set>
#include "store_handlers/qstore_handler.hpp"

class PlayersModel : public QAbstractTableModel {
    Q_OBJECT
public:
    PlayersModel(QStoreHandler & storeHandler, QObject * parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
    void playerAdded(Id id);
    void playerChanged(Id id);
    void playerDeleted(Id id);
    void tournamentReset();
private:
    const int COLUMN_COUNT = 3;
    QStoreHandler & mStoreHandler;
    std::set<Id> mIds;
};
