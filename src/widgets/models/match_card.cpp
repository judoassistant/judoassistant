#include <QPainter>

#include "stores/tournament_store.hpp"
#include "stores/category_store.hpp"
#include "rulesets/ruleset.hpp"
#include "draw_systems/draw_system.hpp"

#include "widgets/models/match_card.hpp"
#include "widgets/colors.hpp"

MatchCard::MatchCard(size_t tatami, const TournamentStore & tournament, const CategoryStore &category, const MatchStore &match)
{
    mTatami = tatami;
    mCategory = QString::fromStdString(category.getName());
    mStatus = match.getStatus();
    mBye = match.isBye();
    mGoldenScore = match.isGoldenScore();

    if (match.getWhitePlayer()) {
        auto whitePlayer = tournament.getPlayer(*match.getWhitePlayer());
        MatchCardPlayerFields fields;
        fields.firstName = QString::fromStdString(whitePlayer.getFirstName());
        fields.lastName = QString::fromStdString(whitePlayer.getLastName());
        fields.club = QString::fromStdString(whitePlayer.getClub());
        fields.country = whitePlayer.getCountry();
        fields.score = match.getWhiteScore();
        mWhitePlayer = std::move(fields);
    }

    if (match.getBluePlayer()) {
        auto bluePlayer = tournament.getPlayer(*match.getBluePlayer());
        MatchCardPlayerFields fields;

        fields.firstName = QString::fromStdString(bluePlayer.getFirstName());
        fields.lastName = QString::fromStdString(bluePlayer.getLastName());
        fields.club = QString::fromStdString(bluePlayer.getClub());
        fields.country = bluePlayer.getCountry();
        fields.score = match.getBlueScore();
        mBluePlayer = std::move(fields);
    }
}

void MatchCard::paintPlayer(MatchCardPlayerFields playerFields, QPainter *painter, QFont &font, int insideWidth, int insideHeight, int columnTwoOffset, int columnThreeOffset, int padding) const {
    { // Draw country name and flag
        // TODO: Draw country name and flag
    }

    { // Draw Name Text
        QRect rect(columnTwoOffset, padding, columnThreeOffset-columnTwoOffset, insideHeight/3 - padding*2);

        painter->save();
        painter->setPen(COLOR_0);
        painter->setBrush(COLOR_0);
        font.setPixelSize((insideHeight/3)/3);
        painter->setFont(font);

        painter->setRenderHint(QPainter::Antialiasing, true);
        auto text = playerFields.lastName.toUpper() + " " + playerFields.firstName.front().toUpper() + ".";
        painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, text);
        painter->restore();
    }

    if (mStatus != MatchStatus::NOT_STARTED) { // Draw Score
        int columnOffset = (insideHeight/3)/2;

        QRect ipponRect(columnThreeOffset, padding, insideWidth-columnThreeOffset, insideHeight/3 - padding*2);
        QRect wazariRect(columnThreeOffset + columnOffset, padding, insideWidth-columnThreeOffset, insideHeight/3 - padding*2);

        int penaltyHeight = (insideHeight/3)/3;
        int penaltyWidth = penaltyHeight*2/3;

        QRect firstPenaltyRect(columnThreeOffset + columnOffset*2, (insideHeight/3)/2-penaltyHeight/2, penaltyWidth, penaltyHeight);
        QRect secondPenaltyRect(columnThreeOffset + columnOffset*2 + penaltyWidth+3, (insideHeight/3)/2-penaltyHeight/2, penaltyWidth, penaltyHeight);

        painter->save();
        painter->setPen(COLOR_0);
        painter->setBrush(COLOR_0);
        font.setPixelSize((insideHeight/3)/3);
        painter->setFont(font);

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawText(ipponRect, Qt::AlignVCenter | Qt::AlignLeft, "0");
        painter->drawText(wazariRect, Qt::AlignVCenter | Qt::AlignLeft, "1");


        painter->setPen(Qt::NoPen);
        painter->setBrush(COLOR_13);

        painter->drawRect(firstPenaltyRect);
        painter->drawRect(secondPenaltyRect);
        painter->restore();
    }

}

void MatchCard::paint(QPainter *painter, const QRect &rect, const QPalette &palette) const {
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, false);

    int padding = 5;
    int lineWidth = 1;
    int insideHeight = rect.height() - padding*2 - lineWidth*2;
    int insideWidth = rect.width() - padding*2 - lineWidth*2;

    QFont font("Noto Sans");

    painter->translate(rect.x()+padding, rect.y()+padding);

    int columnTwoOffset = (insideHeight/3);
    int columnThreeOffset = insideWidth - (insideHeight/3)*2;

    // Draw bounding rect
    QPen pen;
    pen.setWidth(1);
    pen.setColor(COLOR_3);
    painter->setPen(pen);
    painter->setBrush(COLOR_5);
    painter->drawRect(0, 0, insideWidth+lineWidth*2, insideHeight+lineWidth*2);

    // Draw header rect
    {
        int headerHeight = insideHeight - (insideHeight/3)*2;
        painter->save();
        painter->translate(lineWidth, lineWidth);

        font.setPixelSize(headerHeight/3);
        painter->setFont(font);
        painter->setRenderHint(QPainter::Antialiasing, true);

        { // Draw tatami number
            QRect rect(padding, padding, headerHeight-padding*2, headerHeight-padding*2);

            painter->setPen(Qt::NoPen);
            painter->setBrush(COLOR_3);

            painter->drawRect(rect);

            painter->setPen(Qt::white);
            painter->setBrush(Qt::white);

            painter->drawText(rect, Qt::AlignVCenter | Qt::AlignHCenter, QString::number(mTatami+1));
        }

        { // Draw Category Text
            QRect rect(columnTwoOffset, padding, columnThreeOffset-columnTwoOffset, headerHeight-padding*2);

            painter->setPen(COLOR_0);
            painter->setBrush(COLOR_0);

            painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, mCategory);
        }

        if (mStatus != MatchStatus::NOT_STARTED) { // Draw Time
            int pauseRectSize = headerHeight/4;
            QRect pauseRect(columnThreeOffset, headerHeight/2-pauseRectSize/2, pauseRectSize, pauseRectSize);
            painter->setPen(Qt::NoPen);
            painter->setBrush(COLOR_11);
            painter->drawRect(pauseRect);

            QRect textRect(columnThreeOffset + pauseRectSize + padding, padding, insideWidth-(columnThreeOffset + pauseRectSize + padding), headerHeight-padding*2);

            painter->setPen(COLOR_0);
            painter->setBrush(COLOR_0);

            painter->setRenderHint(QPainter::Antialiasing, true);
            // painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, "GS 01:43");
        }
        else { // Draw ETA
            QRect textRect(columnThreeOffset, padding, insideWidth-columnThreeOffset-padding, headerHeight-padding*2);

            painter->setPen(COLOR_0);
            painter->setBrush(COLOR_0);

            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, "~16 min");
        }


        painter->restore();
    }

    // Draw white player rect
    if (mWhitePlayer) {
        painter->save();
        painter->translate(lineWidth, lineWidth+insideHeight-(insideHeight/3)*2);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawRect(0, 0, insideWidth, insideHeight/3);

        paintPlayer(*mWhitePlayer, painter, font, insideWidth, insideHeight, columnTwoOffset, columnThreeOffset, padding);

        painter->restore();
    }
    // painter->setPen(Qt::NoPen);
    // painter->setBrush(COLOR_5::NoPen);

    // Draw blue player rect
    if (mBluePlayer) {
        painter->save();
        painter->translate(lineWidth, lineWidth+insideHeight-(insideHeight/3));
        painter->setPen(Qt::NoPen);
        painter->setBrush(COLOR_10);
        painter->drawRect(0, 0, insideWidth, insideHeight/3);

        paintPlayer(*mBluePlayer, painter, font, insideWidth, insideHeight, columnTwoOffset, columnThreeOffset, padding);

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

