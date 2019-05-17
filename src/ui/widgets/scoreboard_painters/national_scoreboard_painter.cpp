#include <QPainter>

#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/scoreboard_painters/national_scoreboard_painter.hpp"

NationalScoreboardPainter::NationalScoreboardPainter()
    : mFont("Noto Sans")
{
    mFont.setBold(true);
    mFont.setCapitalization(QFont::AllUppercase);
}

void NationalScoreboardPainter::paintEmpty(QPainter &painter, const QRect &rect) {
    // Draw Background
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_SCOREBOARD_BLACK);

    painter.drawRect(rect);
}

void NationalScoreboardPainter::paintIntroduction(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    QRect upperRect(0,0,rect.width(), rect.height()/3);
    QRect middleRect(0,rect.height()/3,rect.width(), rect.height()/3);
    QRect lowerRect(0,2*(rect.height()/3),rect.width(), rect.height() - 2*(rect.height()/3));

    paintIntroductionPlayer(painter, upperRect, params, MatchStore::PlayerIndex::WHITE);
    paintIntroductionPlayer(painter, middleRect, params, MatchStore::PlayerIndex::BLUE);
    paintIntroductionLower(painter, lowerRect, params);
}

void NationalScoreboardPainter::paintNormal(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    QRect upperRect(0,0,rect.width(), rect.height()/3);
    QRect middleRect(0,rect.height()/3,rect.width(), rect.height()/3);
    QRect lowerRect(0,2*(rect.height()/3),rect.width(), rect.height() - 2*(rect.height()/3));

    paintNormalPlayer(painter, upperRect, params, MatchStore::PlayerIndex::WHITE);
    paintNormalPlayer(painter, middleRect, params, MatchStore::PlayerIndex::BLUE);
    paintNormalLower(painter, lowerRect, params);
}

void NationalScoreboardPainter::paintWinner(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    paintNormal(painter, rect, params);
}

void NationalScoreboardPainter::paintIntroductionPlayer(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const PlayerStore &player = (playerIndex == MatchStore::PlayerIndex::WHITE ? params.whitePlayer : params.bluePlayer);

    const int columnOne = PADDING;
    const int columnTwo = columnOne + (rect.width() - columnOne) * 3 / 4;

    painter.save();
    painter.translate(rect.x(), rect.y());

    // Paint background
    QRect boundingRect(0, 0, rect.width(), rect.height());
    painter.setPen(Qt::NoPen);
    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.setBrush(COLOR_SCOREBOARD_WHITE);
    else
        painter.setBrush(COLOR_SCOREBOARD_BLUE);
    painter.drawRect(boundingRect);

    // Set pen color
    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.setPen(COLOR_SCOREBOARD_BLACK);
    else
        painter.setPen(COLOR_SCOREBOARD_WHITE);

    // Calculate rectangles
    auto font = mFont;

    const int innerHeight = rect.height() - PADDING * 2;
    const int nameHeight = innerHeight * 3 / 6;
    const int clubHeight = innerHeight * 2 / 6;

    const int nameOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING : rect.height() - PADDING - nameHeight);
    const int clubOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? rect.height() - PADDING - clubHeight : PADDING);

    QRect clubRect(columnOne, clubOffset, columnTwo - columnOne - PADDING, clubHeight);
    QRect nameRect(columnOne, nameOffset, rect.width() - columnOne - PADDING, nameHeight);

    // Paint club name
    QString clubText = QString::fromStdString(player.getClub());
    font.setPixelSize(clubHeight*8/10);
    painter.setFont(font);

    painter.drawText(clubRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignBottom : Qt::AlignTop) | Qt::AlignLeft, clubText);

    // Paint player name
    font.setPixelSize(nameHeight*7/10);
    painter.setFont(font);
    QString nameText = QString::fromStdString(player.getLastName()) + QString(", ") + QString::fromStdString(player.getFirstName());
    painter.drawText(nameRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignTop : Qt::AlignBottom) | Qt::AlignLeft, nameText);

    painter.restore();
}

void NationalScoreboardPainter::paintIntroductionLower(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    painter.save();
    painter.translate(rect.x(), rect.y());

    // Paint background
    QRect boundingRect(0, 0, rect.width(), rect.height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_SCOREBOARD_BLACK);
    painter.drawRect(boundingRect);

    // Calculate rectangles
    const int innerHeight = rect.height() - 2 * PADDING;
    const int titleHeight = innerHeight * 1/2 - PADDING;
    const int categoryHeight = innerHeight * 1/2;

    QRect titleRect(PADDING, PADDING, rect.width()-PADDING*2, titleHeight);
    QRect categoryRect(PADDING, rect.height() - PADDING - categoryHeight, rect.width()-PADDING*2, categoryHeight);

    // Paint title
    auto font = mFont;
    font.setPixelSize(titleHeight*9/10);
    painter.setFont(font);
    painter.setPen(COLOR_SCOREBOARD_WHITE);

    painter.drawText(titleRect, Qt::AlignBottom | Qt::AlignLeft, QString::fromStdString(params.match.getTitle()));

    // Paint category name
    font.setPixelSize(categoryHeight*9/10);
    painter.setFont(font);

    painter.drawText(categoryRect, Qt::AlignTop | Qt::AlignLeft, QString::fromStdString(params.category.getName()));

    painter.restore();
}

void NationalScoreboardPainter::paintNormalPlayer(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const PlayerStore &player = (playerIndex == MatchStore::PlayerIndex::WHITE ? params.whitePlayer : params.bluePlayer);

    const int columnOne = PADDING;
    const int columnTwo = columnOne + (rect.width() - columnOne) * 3 / 4;

    painter.save();
    painter.translate(rect.x(), rect.y());

    // Paint background
    QRect boundingRect(0, 0, rect.width(), rect.height());
    painter.setPen(Qt::NoPen);
    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.setBrush(COLOR_SCOREBOARD_WHITE);
    else
        painter.setBrush(COLOR_SCOREBOARD_BLUE);
    painter.drawRect(boundingRect);

    // Set pen color
    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.setPen(COLOR_SCOREBOARD_BLACK);
    else
        painter.setPen(COLOR_SCOREBOARD_WHITE);

    // Calculate rectangles
    auto font = mFont;

    const int nameHeight = (rect.height() - PADDING * 3) / 3;
    const int scoreHeight = rect.height() - PADDING * 3 - nameHeight;
    const int clubHeight = nameHeight;

    const int nameOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING : rect.height() - PADDING - nameHeight);
    const int scoreOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING*2+nameHeight : PADDING);
    const int clubOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? rect.height() - PADDING - clubHeight : PADDING);

    QRect clubRect(columnOne, clubOffset, columnTwo - columnOne - PADDING, clubHeight);
    QRect nameRect(columnOne, nameOffset, rect.width() - columnOne - PADDING, nameHeight);
    QRect scoreRect(columnOne, scoreOffset, columnTwo - columnOne - PADDING, scoreHeight);

    // Paint club name
    QString clubText = QString::fromStdString(player.getClub());
    font.setPixelSize(clubHeight*7/10);
    painter.setFont(font);

    painter.drawText(clubRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignBottom : Qt::AlignTop) | Qt::AlignLeft, clubText);

    // Paint player name
    font.setPixelSize(nameHeight*8/10);
    painter.setFont(font);
    QString nameText = QString::fromStdString(player.getLastName()) + QString(", ") + QString::fromStdString(player.getFirstName());
    painter.drawText(nameRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignTop : Qt::AlignBottom) | Qt::AlignLeft, nameText);

    // Score
    font.setPixelSize(scoreHeight*4/5);
    painter.setFont(font);

    const auto &score = params.match.getScore(playerIndex);
    const auto &otherScore = params.match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);
    if (score.hansokuMake == 0 && (score.ippon > 0 || otherScore.hansokuMake == 1))
        painter.drawText(scoreRect, Qt::AlignBottom | Qt::AlignRight, "IPPON");
    else
        painter.drawText(scoreRect, Qt::AlignBottom | Qt::AlignRight, QString::number(score.wazari));

    // Penalties
    const int penaltyHeight = scoreHeight/2;
    const int penaltyWidth = penaltyHeight*3/4;
    QRect firstPenaltyRect(columnTwo, scoreOffset+(scoreHeight-penaltyHeight)/2, penaltyWidth, penaltyHeight);
    QRect secondPenaltyRect(columnTwo + PADDING + penaltyWidth, scoreOffset+(scoreHeight-penaltyHeight)/2, penaltyWidth, penaltyHeight);

    if (score.hansokuMake > 0) {
        painter.setPen(COLOR_SCOREBOARD_BLACK);
        painter.setBrush(COLOR_SCOREBOARD_HANSOKU);
        painter.drawRect(firstPenaltyRect);
    }
    else {
        painter.setPen(COLOR_SCOREBOARD_BLACK);
        painter.setBrush(COLOR_SCOREBOARD_SHIDO);
        if (score.shido > 0)
            painter.drawRect(firstPenaltyRect);
        if (score.shido > 1)
            painter.drawRect(secondPenaltyRect);

    }

    painter.restore();
}

void NationalScoreboardPainter::paintNormalLower(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    const int columnOne = PADDING;
    const int columnThree = columnOne + (rect.width() - columnOne) * 3 / 4;
    const int columnTwo = columnOne + (columnThree - columnOne) * 1 / 3;

    const auto &ruleset = params.category.getRuleset();

    painter.save();
    painter.translate(rect.x(), rect.y());

    // Paint background
    QRect boundingRect(0, 0, rect.width(), rect.height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_SCOREBOARD_BLACK);
    painter.drawRect(boundingRect);

    // Calculate rectangles
    const int innerHeight = rect.height() - 2 * PADDING;
    const int titleHeight = innerHeight * 1/3 - PADDING;
    const int categoryHeight = innerHeight * 1/3;

    QRect titleRect(PADDING, PADDING, columnTwo-PADDING, titleHeight);
    QRect categoryRect(PADDING, rect.height() - PADDING - categoryHeight, columnTwo-PADDING, categoryHeight);

    // Paint title
    auto font = mFont;
    font.setPixelSize(titleHeight*8/10);
    painter.setFont(font);
    painter.setPen(COLOR_SCOREBOARD_WHITE);

    painter.drawText(titleRect, Qt::AlignBottom | Qt::AlignLeft, QString::fromStdString(params.match.getTitle()));

    // Paint category name
    font.setPixelSize(categoryHeight*8/10);
    painter.setFont(font);

    painter.drawText(categoryRect, Qt::AlignTop | Qt::AlignLeft, QString::fromStdString(params.category.getName()));

    // Paint time left
    {
        QRect timeRect(columnTwo, PADDING, columnThree-columnTwo-PADDING, rect.height()-PADDING*2);
        auto time = std::chrono::ceil<std::chrono::seconds>(std::chrono::abs(ruleset.getNormalTime() - params.match.currentDuration(params.masterTime)));
        QString seconds = QString::number((time % std::chrono::minutes(1)).count()).rightJustified(2, '0');
        QString minutes = QString::number(std::chrono::duration_cast<std::chrono::minutes>(time).count());

        font.setPixelSize(rect.height()*6/8);
        painter.setFont(font);

        if (params.match.getStatus() == MatchStatus::UNPAUSED)
            painter.setPen(COLOR_SCOREBOARD_TIME_UNPAUSED);
        else
            painter.setPen(COLOR_SCOREBOARD_TIME_PAUSED);
        painter.drawText(timeRect, Qt::AlignVCenter | Qt::AlignRight, QString("%1:%2").arg(minutes, seconds));
    }

    auto osaekomi = params.match.getOsaekomi();
    if (osaekomi.has_value()) {
        // Paint osaekomi indicator
        QRect osaekomiRect(columnThree, PADDING, rect.width() - columnThree - PADDING, rect.height()-PADDING*2);

        unsigned int seconds = std::chrono::floor<std::chrono::seconds>(params.match.currentOsaekomiTime(params.masterTime)).count();
        QString secondsString = QString::number(seconds).rightJustified(2, '0');

        font.setPixelSize(rect.height()*5/8);
        painter.setFont(font);

        painter.setPen(COLOR_SCOREBOARD_OSAEKOMI);
        painter.drawText(osaekomiRect, Qt::AlignBottom | Qt::AlignRight, secondsString);
    }
    else if (params.match.isGoldenScore()) {
        // Paint golden score indicator
        QRect goldenScoreRect(columnThree, PADDING, rect.width() - columnThree - PADDING, rect.height()-PADDING*2);

        font.setPixelSize(rect.height()*1/3);
        painter.setFont(font);

        painter.setPen(COLOR_SCOREBOARD_GOLDEN_SCORE);
        painter.drawText(goldenScoreRect, Qt::AlignVCenter | Qt::AlignLeft, "GS");
    }

    painter.restore();
}

