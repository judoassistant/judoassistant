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
    paintNormal(painter, rect, params);
    // paintIntroductionPlayer(painter, params, MatchStore::PlayerIndex::WHITE);
    // paintIntroductionPlayer(painter, params, MatchStore::PlayerIndex::BLUE);
    // paintIntroductionLower(painter, params);
}

void NationalScoreboardPainter::paintNormal(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    painter.save();
    painter.translate(rect.topLeft());
    paintNormalPlayer(painter, params, MatchStore::PlayerIndex::WHITE);
    paintNormalPlayer(painter, params, MatchStore::PlayerIndex::BLUE);
    paintNormalLower(painter, params);
    painter.restore();
}

void NationalScoreboardPainter::paintWinner(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    paintNormal(painter, rect, params);
}

void NationalScoreboardPainter::paintIntroductionPlayer(QPainter &painter, const ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    // const PlayerStore &player = (playerIndex == MatchStore::PlayerIndex::WHITE ? params.whitePlayer : params.bluePlayer);

    // painter.save();
    // painter.translate(rect.x(), rect.y());

    // // Paint background
    // QRect boundingRect(0, 0, rect.width(), rect.height());
    // painter.setPen(Qt::NoPen);
    // if (playerIndex == MatchStore::PlayerIndex::WHITE)
    //     painter.setBrush(COLOR_SCOREBOARD_WHITE);
    // else
    //     painter.setBrush(COLOR_SCOREBOARD_BLUE);
    // painter.drawRect(boundingRect);

    // // Set pen color
    // if (playerIndex == MatchStore::PlayerIndex::WHITE)
    //     painter.setPen(COLOR_SCOREBOARD_BLACK);
    // else
    //     painter.setPen(COLOR_SCOREBOARD_WHITE);

    // // Calculate rectangles
    // auto font = mFont;

    // const int innerHeight = rect.height() - PADDING * 2;
    // const int nameHeight = innerHeight * 3 / 6;
    // const int clubHeight = innerHeight * 2 / 6;

    // const int nameOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING : rect.height() - PADDING - nameHeight);
    // const int clubOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? rect.height() - PADDING - clubHeight : PADDING);

    // QRect clubRect(mColumnOne, clubOffset, mColumnThree - mColumnOne - PADDING, clubHeight);
    // QRect nameRect(mColumnOne, nameOffset, rect.width() - mColumnOne - PADDING, nameHeight);

    // // Paint club name
    // QString clubText = QString::fromStdString(player.getClub());
    // font.setPixelSize(clubHeight*8/10);
    // painter.setFont(font);

    // painter.drawText(clubRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignBottom : Qt::AlignTop) | Qt::AlignLeft, clubText);

    // // Paint player name
    // font.setPixelSize(nameHeight*7/10);
    // painter.setFont(font);
    // QString nameText = QString::fromStdString(player.getLastName()) + QString(", ") + QString::fromStdString(player.getFirstName());
    // painter.drawText(nameRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignTop : Qt::AlignBottom) | Qt::AlignLeft, nameText);

    // painter.restore();
}

void NationalScoreboardPainter::paintIntroductionLower(QPainter &painter, const ScoreboardPainterParams &params) {
    // painter.save();
    // painter.translate(rect.x(), rect.y());

    // // Paint background
    // QRect boundingRect(0, 0, rect.width(), rect.height());
    // painter.setPen(Qt::NoPen);
    // painter.setBrush(COLOR_SCOREBOARD_BLACK);
    // painter.drawRect(boundingRect);

    // // Calculate rectangles
    // const int innerHeight = rect.height() - 2 * PADDING;
    // const int titleHeight = innerHeight * 1/2 - PADDING;
    // const int categoryHeight = innerHeight * 1/2;

    // QRect titleRect(PADDING, PADDING, rect.width()-PADDING*2, titleHeight);
    // QRect categoryRect(PADDING, rect.height() - PADDING - categoryHeight, rect.width()-PADDING*2, categoryHeight);

    // // Paint title
    // auto font = mFont;
    // font.setPixelSize(titleHeight*9/10);
    // painter.setFont(font);
    // painter.setPen(COLOR_SCOREBOARD_WHITE);

    // painter.drawText(titleRect, Qt::AlignBottom | Qt::AlignLeft, QString::fromStdString(params.match.getTitle()));

    // // Paint category name
    // font.setPixelSize(categoryHeight*9/10);
    // painter.setFont(font);

    // painter.drawText(categoryRect, Qt::AlignTop | Qt::AlignLeft, QString::fromStdString(params.category.getName()));

    // painter.restore();
}

void NationalScoreboardPainter::paintNormalPlayer(QPainter &painter, const ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const PlayerStore &player = (playerIndex == MatchStore::PlayerIndex::WHITE ? params.whitePlayer : params.bluePlayer);

    // Paint background
    painter.setPen(Qt::NoPen);
    if (playerIndex == MatchStore::PlayerIndex::WHITE) {
        painter.setBrush(COLOR_SCOREBOARD_WHITE);
        painter.drawRect(mWhiteRect);
    }
    else {
        painter.setBrush(COLOR_SCOREBOARD_BLUE);
        painter.drawRect(mBlueRect);
    }

    // Set pen color
    painter.setPen(playerIndex == MatchStore::PlayerIndex::WHITE ? COLOR_SCOREBOARD_BLACK : COLOR_SCOREBOARD_WHITE);

    // Calculate rectangles
    auto font = mFont;

    // Paint club name
    QString clubText = QString::fromStdString(player.getClub());
    font.setPixelSize(mNormalClubFontSize);
    painter.setFont(font);

    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.drawText(mNormalWhiteClubRect, Qt::AlignBottom | Qt::AlignLeft, clubText);
    else
        painter.drawText(mNormalBlueClubRect, Qt::AlignTop | Qt::AlignLeft, clubText);

    // Paint player name
    font.setPixelSize(mNormalNameFontSize);
    painter.setFont(font);
    QString nameText = QString::fromStdString(player.getLastName()) + QString(", ") + QString::fromStdString(player.getFirstName());

    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.drawText(mNormalWhiteNameRect, Qt::AlignTop | Qt::AlignLeft, nameText);
    else
        painter.drawText(mNormalBlueNameRect, Qt::AlignBottom | Qt::AlignLeft, nameText);

    // Score
    font.setPixelSize(mScoreFontSize);
    painter.setFont(font);

    const auto &score = params.match.getScore(playerIndex);
    const auto &otherScore = params.match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);

    QString scoreText;
    if (score.hansokuMake == 0 && (score.ippon > 0 || otherScore.hansokuMake == 1))
        scoreText = "IPPON";
    else
        scoreText = QString::number(score.wazari);

    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.drawText(mWhiteScoreRect, Qt::AlignBottom | Qt::AlignRight, scoreText);
    else
        painter.drawText(mBlueScoreRect, Qt::AlignBottom | Qt::AlignRight, scoreText);

    // Penalties
    if (score.hansokuMake > 0) {
        painter.setPen(COLOR_SCOREBOARD_BLACK);
        painter.setBrush(COLOR_SCOREBOARD_HANSOKU);

        painter.drawRect(playerIndex == MatchStore::PlayerIndex::WHITE ? mWhiteFirstPenaltyRect : mBlueFirstPenaltyRect);
    }
    else {
        painter.setPen(COLOR_SCOREBOARD_BLACK);
        painter.setBrush(COLOR_SCOREBOARD_SHIDO);
        if (score.shido > 0)
            painter.drawRect(playerIndex == MatchStore::PlayerIndex::WHITE ? mWhiteFirstPenaltyRect : mBlueFirstPenaltyRect);
        if (score.shido > 1)
            painter.drawRect(playerIndex == MatchStore::PlayerIndex::WHITE ? mWhiteSecondPenaltyRect : mBlueSecondPenaltyRect);

    }
}

void NationalScoreboardPainter::paintNormalLower(QPainter &painter, const ScoreboardPainterParams &params) {
    const auto &ruleset = params.category.getRuleset();

    // Paint background
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_SCOREBOARD_BLACK);
    painter.drawRect(mLowerRect);

    // Paint title
    auto font = mFont;
    font.setPixelSize(mNormalTitleFontSize);
    painter.setFont(font);
    painter.setPen(COLOR_SCOREBOARD_WHITE);
    painter.setBrush(Qt::NoBrush);

    painter.drawText(mNormalTitleRect, Qt::AlignBottom | Qt::AlignLeft, QString::fromStdString(params.match.getTitle()));

    // Paint category name
    font.setPixelSize(mNormalCategoryFontSize);
    painter.setFont(font);

    painter.drawText(mNormalCategoryRect, Qt::AlignTop | Qt::AlignLeft, QString::fromStdString(params.category.getName()));

    // Paint time left
    {
        auto time = std::chrono::ceil<std::chrono::seconds>(std::chrono::abs(ruleset.getNormalTime() - params.match.currentDuration(params.masterTime)));
        QString seconds = QString::number((time % std::chrono::minutes(1)).count()).rightJustified(2, '0');
        QString minutes = QString::number(std::chrono::duration_cast<std::chrono::minutes>(time).count());

        font.setPixelSize(mDurationFontSize);
        painter.setFont(font);

        if (params.match.getStatus() == MatchStatus::UNPAUSED)
            painter.setPen(COLOR_SCOREBOARD_TIME_UNPAUSED);
        else
            painter.setPen(COLOR_SCOREBOARD_TIME_PAUSED);
        painter.drawText(mDurationRect, Qt::AlignVCenter | Qt::AlignRight, QString("%1:%2").arg(minutes, seconds));
    }

    auto osaekomi = params.match.getOsaekomi();
    if (osaekomi.has_value()) {
        // Paint osaekomi indicator
        unsigned int seconds = std::chrono::floor<std::chrono::seconds>(params.match.currentOsaekomiTime(params.masterTime)).count();
        QString secondsString = QString::number(seconds).rightJustified(2, '0');

        font.setPixelSize(mOsaekomiFontSize);
        painter.setFont(font);

        painter.setPen(COLOR_SCOREBOARD_OSAEKOMI);
        painter.drawText(mOsaekomiRect, Qt::AlignBottom | Qt::AlignRight, secondsString);
    }
    else if (params.match.isGoldenScore()) {
        // Paint golden score indicator
        font.setPixelSize(mGoldenScoreFontSize);
        painter.setFont(font);

        painter.setPen(COLOR_SCOREBOARD_GOLDEN_SCORE);
        painter.drawText(mGoldenScoreRect, Qt::AlignVCenter | Qt::AlignLeft, "GS");
    }
}

void NationalScoreboardPainter::resizeEvent(const QRect &rect) {
    mWhiteRect = QRect(0,0,rect.width(), rect.height()/3);
    mBlueRect = QRect(0,rect.height()/3,rect.width(), rect.height()/3);
    mLowerRect = QRect(0,2*(rect.height()/3),rect.width(), rect.height() - 2*(rect.height()/3));

    const int mColumnOne = PADDING;
    const int mColumnThree = mColumnOne + (rect.width() - mColumnOne) * 3 / 4;
    const int mColumnTwo = mColumnOne + (mColumnThree - mColumnOne) * 3 / 6;

    // Calculate player rectangles
    {
        const int innerHeight = mWhiteRect.height() - PADDING * 2;
        const int nameHeight = innerHeight / 3;
        const int clubHeight = nameHeight;
        const int scoreHeight = innerHeight - PADDING - nameHeight;
        const int penaltyHeight = scoreHeight / 2;
        const int penaltyWidth = penaltyHeight * 3/4;

        const int whiteNameOffset = PADDING;
        const int blueNameOffset = mWhiteRect.height() + mBlueRect.height() - PADDING - nameHeight;

        const int whiteClubOffset = mWhiteRect.height() - PADDING - clubHeight;
        const int blueClubOffset = mWhiteRect.height() + PADDING;

        const int whiteScoreOffset = PADDING * 2 + nameHeight;
        const int blueScoreOffset = mWhiteRect.height() + PADDING;

        const int whitePenaltyOffset = whiteScoreOffset + (scoreHeight - penaltyHeight)/2;
        const int bluePenaltyOffset = blueScoreOffset + (scoreHeight - penaltyHeight)/2;

        mWhiteFirstPenaltyRect = QRect(mColumnThree, whitePenaltyOffset, penaltyWidth, penaltyHeight);
        mWhiteSecondPenaltyRect = QRect(mColumnThree + PADDING + penaltyWidth, whitePenaltyOffset, penaltyWidth, penaltyHeight);

        mBlueFirstPenaltyRect = QRect(mColumnThree, bluePenaltyOffset, penaltyWidth, penaltyHeight);
        mBlueSecondPenaltyRect = QRect(mColumnThree + PADDING + penaltyWidth, bluePenaltyOffset, penaltyWidth, penaltyHeight);

        mNormalClubFontSize = clubHeight * 7/10;
        mNormalNameFontSize = nameHeight * 8/10;
        mScoreFontSize = scoreHeight * 4/5;

        // mIntroductionWhiteClubRect(mColumnOne, whiteClubOffset, mColumnThree - mColumnOne - PADDING, clubHeight);
        // mIntroductionBlueClubRect(mColumnOne, blueClubOffset, mColumnThree - mColumnOne - PADDING, clubHeight);

        // mIntroductionWhiteNameRect(mColumnOne, whiteNameOffset, whiteRect.width() - mColumnOne - PADDING, nameHeight);
        // mIntroductionBlueNameRect(mColumnOne, blueNameOffset, blueRect.width() - mColumnOne - PADDING, nameHeight);

        mNormalWhiteClubRect = QRect(mColumnOne, whiteClubOffset, mColumnThree - mColumnOne - PADDING, clubHeight);
        mNormalBlueClubRect = QRect(mColumnOne, blueClubOffset, mColumnThree - mColumnOne - PADDING, clubHeight);

        mNormalWhiteNameRect = QRect(mColumnOne, whiteNameOffset, mWhiteRect.width() - mColumnOne - PADDING, nameHeight);
        mNormalBlueNameRect = QRect(mColumnOne, blueNameOffset, mBlueRect.width() - mColumnOne - PADDING, nameHeight);


        mWhiteScoreRect = QRect(mColumnOne, whiteScoreOffset, mColumnThree - mColumnOne - PADDING, scoreHeight);
        mBlueScoreRect = QRect(mColumnOne, blueScoreOffset, mColumnThree - mColumnOne - PADDING, scoreHeight);
    }


    // Calculate lower rectangles
    // mIntroductionTitleRect(PADDING, PADDING, rect.width()-PADDING*2, titleHeight);
    // mIntroductionCategoryRect(PADDING, rect.height() - PADDING - categoryHeight, rect.width()-PADDING*2, categoryHeight);

    const int titleHeight = (mLowerRect.height() - 3 * PADDING) / 2;
    const int categoryHeight = titleHeight;
    const int durationHeight = mLowerRect.height() - PADDING * 2;
    const int osaekomiHeight = durationHeight;
    const int goldenScoreHeight = durationHeight;

    const int titleOffset = mWhiteRect.height() + mBlueRect.height() + PADDING;
    const int durationOffset = titleOffset;
    const int osaekomiOffset = titleOffset;
    const int categoryOffset = rect.height() - PADDING - categoryHeight;
    const int goldenScoreOffset = titleOffset;

    mNormalCategoryFontSize = categoryHeight * 3/6;
    mNormalTitleFontSize = titleHeight * 3/6;
    mDurationFontSize = mLowerRect.height() * 6/8;
    mOsaekomiFontSize = mLowerRect.height() * 5/8;
    mGoldenScoreFontSize = mLowerRect.height() * 1/3;

    mNormalTitleRect = QRect(PADDING, titleOffset, rect.width()-PADDING*2, titleHeight);
    mNormalCategoryRect = QRect(PADDING, categoryOffset, rect.width()-PADDING*2, categoryHeight);

    mDurationRect = QRect(mColumnTwo, durationOffset, mColumnThree-mColumnTwo-PADDING, durationHeight);
    mOsaekomiRect = QRect(mColumnThree, osaekomiOffset, rect.width() - mColumnThree - PADDING, osaekomiHeight);
    mGoldenScoreRect = QRect(mColumnThree, goldenScoreOffset, rect.width() - mColumnThree - PADDING, goldenScoreHeight);
}

