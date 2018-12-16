#include "widgets/delegates/match_card_delegate.hpp"
#include "widgets/models/match_card.hpp"

MatchCardDelegate::MatchCardDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{}

void MatchCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data().canConvert<MatchCard>()) {
        MatchCard matchCard = qvariant_cast<MatchCard>(index.data());
        // if (option.state & QStyle::State_Selected)
        //     painter->fillRect(option.rect, option.palette.highlight());

        // TODO: Add padding here instead of in match card
        matchCard.paint(painter, option.rect, option.palette);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize MatchCardDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data().canConvert<MatchCard>()) {
        MatchCard matchCard = qvariant_cast<MatchCard>(index.data());
        return matchCard.sizeHint();
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

