#include <QPainter>

#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"
#include "ui/models/match_card.hpp"
#include "ui/widgets/colors.hpp"

MatchCard::MatchCard(const TournamentStore & tournament, const CategoryStore &category, const MatchStore &match, std::chrono::milliseconds masterTime)
{
    setCategory(tournament, category, match);
    setWhitePlayer(tournament, match);
    setBluePlayer(tournament, match);
    setMatch(category, match, masterTime);
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

        if (playerFields.score.ippon != 0)
            painter->drawText(ipponRect, Qt::AlignVCenter | Qt::AlignLeft, QString::number(playerFields.score.ippon));
        painter->drawText(wazariRect, Qt::AlignVCenter | Qt::AlignLeft, QString::number(playerFields.score.wazari));

        painter->setPen(Qt::NoPen);
        if (playerFields.score.hansokuMake) {
            painter->setBrush(COLOR_11);
            painter->drawRect(firstPenaltyRect);
        }
        else {
            painter->setBrush(COLOR_13);
            if (playerFields.score.shido > 0)
                painter->drawRect(firstPenaltyRect);
            if (playerFields.score.shido > 1)
                painter->drawRect(secondPenaltyRect);
        }

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

        if (mTatami) { // Draw tatami number
            QRect rect(padding, padding, headerHeight-padding*2, headerHeight-padding*2);

            painter->setPen(Qt::NoPen);
            painter->setBrush(COLOR_3);

            painter->drawRect(rect);

            painter->setPen(Qt::white);
            painter->setBrush(Qt::white);

            painter->drawText(rect, Qt::AlignVCenter | Qt::AlignHCenter, QString::number((*mTatami)+1));
        }

        { // Draw Category Text
            QRect rect(columnTwoOffset, padding, columnThreeOffset-columnTwoOffset, headerHeight-padding*2);

            painter->setPen(COLOR_0);
            painter->setBrush(COLOR_0);

            painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, mCategory);
        }

        int pauseRectSize = headerHeight/4;
        QRect pauseRect(columnThreeOffset, headerHeight/2-pauseRectSize/2, pauseRectSize, pauseRectSize);
        QRect textRect(columnThreeOffset + pauseRectSize + padding, padding, insideWidth-(columnThreeOffset + pauseRectSize + padding), headerHeight-padding*2);
        if (mBye) {
            painter->setPen(COLOR_0);
            painter->setBrush(COLOR_0);
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, QString("bye"));
        }
        else if (mStatus != MatchStatus::NOT_STARTED && mStatus != MatchStatus::FINISHED) { // Draw Time
            painter->setPen(Qt::NoPen);
            if (mStatus != MatchStatus::UNPAUSED)
                painter->setBrush(COLOR_13);
            else
                painter->setBrush(COLOR_14);
            painter->drawRect(pauseRect);

            painter->setPen(COLOR_0);
            painter->setBrush(COLOR_0);

            QString seconds = QString::number((mTime % std::chrono::minutes(1)).count()).rightJustified(2, '0');
            QString minutes = QString::number(std::chrono::duration_cast<std::chrono::minutes>(mTime).count());

            if (mGoldenScore)
                painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, QString("GS %1:%2").arg(minutes).arg(seconds));
            else
                painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, QString("%1:%2").arg(minutes).arg(seconds));
        }
        else { // Draw ETA
            // TODO: Calculate ETA for matches
            // painter->setPen(COLOR_0);
            // painter->setBrush(COLOR_0);

            // painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, "~16 min");
        }


        painter->restore();
    }

    // Draw white player rect
    painter->save();
    painter->translate(lineWidth, lineWidth+insideHeight-(insideHeight/3)*2);
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);
    painter->drawRect(0, 0, insideWidth, insideHeight/3);

    if (mWhitePlayer)
        paintPlayer(*mWhitePlayer, painter, font, insideWidth, insideHeight, columnTwoOffset, columnThreeOffset, padding);

    painter->restore();
    // painter->setPen(Qt::NoPen);
    // painter->setBrush(COLOR_5::NoPen);

    // Draw blue player rect
    painter->save();
    painter->translate(lineWidth, lineWidth+insideHeight-(insideHeight/3));
    painter->setPen(Qt::NoPen);
    painter->setBrush(COLOR_10);
    painter->drawRect(0, 0, insideWidth, insideHeight/3);

    if (mBluePlayer)
        paintPlayer(*mBluePlayer, painter, font, insideWidth, insideHeight, columnTwoOffset, columnThreeOffset, padding);

    painter->restore();

    painter->restore();
}

QSize MatchCard::sizeHint() const {
    return QSize(WIDTH_HINT, HEIGHT_HINT);
}

void MatchCard::setCategory(const TournamentStore &tournament, const CategoryStore &category, const MatchStore &match) {
    mCategory = QString::fromStdString(category.getName());

    const auto &tatamis = tournament.getTatamis();

    auto location = category.getLocation(match.getType());
    if (location) {
        auto tatamiLocation = location->sequentialGroup.concurrentGroup.tatami;
        mTatami = tatamis.getIndex(tatamiLocation);
    }
    else {
        mTatami = std::nullopt;
    }
}

void MatchCard::setWhitePlayer(const TournamentStore & tournament, const MatchStore &match) {
    if (!match.getWhitePlayer()) {
        mWhitePlayer = std::nullopt;
        return;
    }

    auto whitePlayer = tournament.getPlayer(*match.getWhitePlayer());
    MatchCardPlayerFields fields;
    fields.firstName = QString::fromStdString(whitePlayer.getFirstName());
    fields.lastName = QString::fromStdString(whitePlayer.getLastName());
    fields.club = QString::fromStdString(whitePlayer.getClub());
    fields.country = whitePlayer.getCountry();
    fields.score = match.getWhiteScore();
    mWhitePlayer = std::move(fields);
}

void MatchCard::setBluePlayer(const TournamentStore & tournament, const MatchStore &match) {
    if (!match.getBluePlayer()) {
        mBluePlayer = std::nullopt;
        return;
    }

    auto bluePlayer = tournament.getPlayer(*match.getBluePlayer());
    MatchCardPlayerFields fields;

    fields.firstName = QString::fromStdString(bluePlayer.getFirstName());
    fields.lastName = QString::fromStdString(bluePlayer.getLastName());
    fields.club = QString::fromStdString(bluePlayer.getClub());
    fields.country = bluePlayer.getCountry();
    fields.score = match.getBlueScore();
    mBluePlayer = std::move(fields);
}

void MatchCard::setMatch(const CategoryStore &category, const MatchStore &match, std::chrono::milliseconds masterTime) {
    mStatus = match.getStatus();
    mBye = match.isBye();
    mGoldenScore = match.isGoldenScore();
    mTime = std::chrono::ceil<std::chrono::seconds>(std::chrono::abs(category.getRuleset().getNormalTime() - match.currentDuration(masterTime)));
}

