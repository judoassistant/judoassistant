#pragma once

#include "core/core.hpp"
#include "core/id.hpp"

#include <QStyledItemDelegate>
#include <QPainter>

class StoreManager;

class MatchDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    MatchDelegate(const StoreManager &storeManager, QWidget *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    const StoreManager &mStoreManager;
};

Q_DECLARE_METATYPE(CombinedId);

