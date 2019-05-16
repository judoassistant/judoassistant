#pragma once

#include <stack>
#include <unordered_set>
#include <QMetaObject>
#include <QWidget>
#include <QStyledItemDelegate>

#include "core/core.hpp"

class StoreManager;
class QTableView;

class PreferredDrawSystemsWidget : public QWidget {
    Q_OBJECT
public:
    PreferredDrawSystemsWidget(StoreManager &storeManager, QWidget *parent = nullptr);

private:
    StoreManager &mStoreManager;
    QTableView *mTableView;

    void showContextMenu(const QPoint &pos);
    void insertRowBelow();
    void eraseRow();
    std::unordered_set<std::size_t> selectedRows();
};

class DrawSystemPreferenceDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    DrawSystemPreferenceDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void commitComboBox();
};

