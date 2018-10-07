#pragma once

#include <QAbstractTableModel>
#include <QItemSelection>
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

    std::vector<Id> getPlayers(const QItemSelection &selection) const;
    Id getPlayer(int row) const;

    int getRow(Id id) const;
    std::vector<int> getRows(std::vector<Id> id) const;
public slots:
    void playersAdded(std::vector<Id> ids);
    void playersChanged(std::vector<Id> ids);
    void playersAboutToBeErased(std::vector<Id> ids);
    void playersAboutToBeReset();
    void playersReset();
    void tournamentReset();
private:
    const int COLUMN_COUNT = 7;
    QStoreHandler & mStoreHandler;
    std::set<Id> mIds;
};
