#pragma once

#include <QAbstractTableModel>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include <set>
#include "store_handlers/qstore_handler.hpp"

// TODO: Correctly sort by name
class CategoriesModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CategoriesModel(QStoreHandler & storeHandler, QObject * parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::vector<CategoryId> getCategories(const QItemSelection &selection) const;
    CategoryId getCategory(int row) const;

    int getRow(CategoryId id) const;
    std::vector<int> getRows(std::vector<CategoryId> id) const;
public slots:
    void categoriesAdded(std::vector<CategoryId> ids);
    void categoriesChanged(std::vector<CategoryId> ids);
    void categoriesAboutToBeErased(std::vector<CategoryId> ids);
    void categoriesAboutToBeReset();
    void categoriesReset();
    void tournamentReset();
private:
    const int COLUMN_COUNT = 4;
    QStoreHandler & mStoreHandler;
    std::set<CategoryId> mIds;
};

class CategoriesProxyModel : public QSortFilterProxyModel {
public:
    CategoriesProxyModel(QStoreHandler &storeHandler, QObject *parent);
    std::vector<CategoryId> getCategories(const QItemSelection &selection) const;

private:
    CategoriesModel *mModel;
};

