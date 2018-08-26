#pragma once
#include <QAbstractTableModel>
#include "stores/qtournament_store.hpp"

class PlayersModel : public QAbstractTableModel {
    Q_OBJECT
public:
    PlayersModel(std::unique_ptr<QTournamentStore> & tournament, QObject * parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
    void playerAdded(Id id);
    void playerChanged(Id id);
    void playerDeleted(Id id);
private:
    std::unique_ptr<QTournamentStore> & mTournament;
};
