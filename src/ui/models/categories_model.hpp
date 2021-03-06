#pragma once

#include <QAbstractTableModel>
#include <QItemSelection>
#include <QMetaObject>
#include <QSortFilterProxyModel>
#include <set>
#include <stack>

#include "core/id.hpp"

class StoreManager;

class CategoriesModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CategoriesModel(StoreManager & storeManager, QObject * parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::vector<CategoryId> getCategories(const QItemSelection &selection) const;
    CategoryId getCategory(int row) const;

    int getRow(CategoryId id) const;
    std::vector<int> getRows(std::vector<CategoryId> id) const;

public slots:
    void categoriesAdded(const std::vector<CategoryId> &categoryIds);
    void categoriesChanged(const std::vector<CategoryId> &categoryIds);
    void categoriesAboutToBeErased(const std::vector<CategoryId> &categoryIds);
    void categoriesAboutToBeReset();
    void categoriesReset();
    void matchesReset(const std::vector<CategoryId> &categoryIds);
    void categoryPlayersChanged(CategoryId categoryId);
    void tournamentAboutToBeReset();
    void tournamentReset();

private:
    const int COLUMN_COUNT = 5;
    StoreManager & mStoreManager;
    std::set<CategoryId> mIds;
    std::stack<QMetaObject::Connection> mConnections;
};

class CategoriesProxyModel : public QSortFilterProxyModel {
public:
    CategoriesProxyModel(StoreManager &storeManager, QObject *parent);
    std::vector<CategoryId> getCategories(const QItemSelection &selection) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    StoreManager &mStoreManager;
    CategoriesModel *mModel;
};

