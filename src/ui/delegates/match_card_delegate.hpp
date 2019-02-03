#pragma once

#include <QStyledItemDelegate>
#include <QPainter>

class MatchCardDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    MatchCardDelegate(QWidget *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
};

