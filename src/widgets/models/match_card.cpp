#include <QPainter>

#include "widgets/models/match_card.hpp"
#include "widgets/colors.hpp"

MatchCard::MatchCard(std::optional<MatchCardPlayerFields> whitePlayer, std::optional<MatchCardPlayerFields> bluePlayer)
    : mWhitePlayer(whitePlayer)
    , mBluePlayer(bluePlayer)
{}

void MatchCard::paint(QPainter *painter, const QRect &rect, const QPalette &palette) const {
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, false);

    int padding = 5;
    int lineWidth = 1;
    int insideHeight = rect.height() - padding*2 - lineWidth*2;
    int insideWidth = rect.width() - padding*2 - lineWidth*2;

    painter->translate(rect.x()+padding, rect.y()+padding);

    // Draw bounding rect
    painter->setPen(COLOR_4);
    painter->setBrush(COLOR_5);
    painter->drawRect(0, 0, insideWidth+lineWidth*2, insideHeight+lineWidth*2);

    // Draw header rect
    {
        painter->save();
        painter->translate(lineWidth, lineWidth);
        painter->restore();
    }

    // Draw white player rect
    {
        painter->save();
        painter->translate(lineWidth, lineWidth+insideHeight-(insideHeight/3)*2);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawRect(0, 0, insideWidth, insideHeight/3);
        painter->restore();
    }
    // painter->setPen(Qt::NoPen);
    // painter->setBrush(COLOR_5::NoPen);

    // Draw blue player rect
    {
        painter->save();
        painter->translate(lineWidth, lineWidth+insideHeight-(insideHeight/3));
        painter->setPen(Qt::NoPen);
        painter->setBrush(COLOR_10);
        painter->drawRect(0, 0, insideWidth, insideHeight/3);
        painter->restore();
    }

    painter->restore();
}

QSize MatchCard::sizeHint() {
    return QSize(WIDTH_HINT, HEIGHT_HINT);
}

MatchCardDelegate::MatchCardDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{}

void MatchCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data().canConvert<MatchCard>()) {
        MatchCard matchCard = qvariant_cast<MatchCard>(index.data());
        // if (option.state & QStyle::State_Selected)
        //     painter->fillRect(option.rect, option.palette.highlight());

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

