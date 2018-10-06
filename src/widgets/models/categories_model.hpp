#pragma once
#include <QAbstractTableModel>
#include <set>
#include "store_handlers/qstore_handler.hpp"

class CategoriesModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CategoriesModel(QStoreHandler & storeHandler, QObject * parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
    void categoryAdded(Id id);
    void categoryChanged(Id id);
    void categoryDeleted(Id id);
    void tournamentReset();
private:
    const int COLUMN_COUNT = 4;
    QStoreHandler & mStoreHandler;
    std::set<Id> mIds;
};
