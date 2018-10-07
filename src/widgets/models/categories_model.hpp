#pragma once

#include <QAbstractTableModel>
#include <QItemSelection>
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

    std::vector<Id> getCategories(const QItemSelection &selection) const;
    Id getCategory(int row) const;

    int getRow(Id id) const;
    std::vector<int> getRows(std::vector<Id> id) const;
public slots:
    void categoriesAdded(std::vector<Id> ids);
    void categoriesChanged(std::vector<Id> ids);
    void categoriesAboutToBeErased(std::vector<Id> ids);
    void categoriesAboutToBeReset();
    void categoriesReset();
    void tournamentReset();
private:
    const int COLUMN_COUNT = 4;
    QStoreHandler & mStoreHandler;
    std::set<Id> mIds;
};
