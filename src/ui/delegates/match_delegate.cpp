#include "ui/delegates/match_delegate.hpp"
#include "ui/widgets/graphics_items/match_graphics_item.hpp"

constexpr int MARGIN = 4;

MatchDelegate::MatchDelegate(const StoreManager &storeManager, QWidget *parent)
    : QStyledItemDelegate(parent)
    , mStoreManager(storeManager)
{}

void MatchDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data().canConvert<CombinedId>()) {
        CombinedId combinedId = qvariant_cast<CombinedId>(index.data());

        QRect drawRect(option.rect.x() + MARGIN, option.rect.y() + MARGIN, option.rect.width() - 2 * MARGIN, option.rect.height() - 2 * MARGIN);
        if (option.state & QStyle::State_Selected)
            painter->fillRect(drawRect, option.palette.highlight());

        QStyleOptionGraphicsItem styleOption;
        MatchGraphicsItem item(mStoreManager, combinedId.getCategoryId(), combinedId.getMatchId(), drawRect);
        item.paint(painter, &styleOption, nullptr);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize MatchDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data().canConvert<CombinedId>()) {
        return QSize(MatchGraphicsItem::WIDTH_HINT + 2 * MARGIN, MatchGraphicsItem::HEIGHT_HINT + 2 * MARGIN);
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

