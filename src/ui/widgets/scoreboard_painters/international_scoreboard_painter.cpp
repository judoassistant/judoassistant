#include <QPainter>

#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/scoreboard_painters/international_scoreboard_painter.hpp"

InternationalScoreboardPainter::InternationalScoreboardPainter()
    : mFont("Noto Sans")
{
    mFont.setBold(true);
    mFont.setCapitalization(QFont::AllUppercase);
}

void InternationalScoreboardPainter::paintEmpty(QPainter &painter, const QRect &rect) {
    // Draw Background
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_SCOREBOARD_BLACK);

    painter.drawRect(rect);
}

void InternationalScoreboardPainter::paintIntroduction(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    QRect upperRect(0,0,rect.width(), rect.height()/3);
    QRect middleRect(0,rect.height()/3,rect.width(), rect.height()/3);
    QRect lowerRect(0,2*(rect.height()/3),rect.width(), rect.height() - 2*(rect.height()/3));

    paintIntroductionPlayer(painter, upperRect, params, MatchStore::PlayerIndex::WHITE);
    paintIntroductionPlayer(painter, middleRect, params, MatchStore::PlayerIndex::BLUE);
    paintIntroductionLower(painter, lowerRect, params);
}

void InternationalScoreboardPainter::paintNormal(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    QRect upperRect(0,0,rect.width(), rect.height()/3);
    QRect middleRect(0,rect.height()/3,rect.width(), rect.height()/3);
    QRect lowerRect(0,2*(rect.height()/3),rect.width(), rect.height() - 2*(rect.height()/3));

    paintNormalPlayer(painter, upperRect, params, MatchStore::PlayerIndex::WHITE);
    paintNormalPlayer(painter, middleRect, params, MatchStore::PlayerIndex::BLUE);
    paintNormalLower(painter, lowerRect, params);
}

void InternationalScoreboardPainter::paintWinner(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    paintNormal(painter, rect, params);
}

void InternationalScoreboardPainter::paintIntroductionPlayer(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const PlayerStore &player = (playerIndex == MatchStore::PlayerIndex::WHITE ? params.whitePlayer : params.bluePlayer);
    const int flagHeight = (rect.height() - PADDING * 3) / 2;
    const int flagWidth = flagHeight * 4 / 3;

    const int columnOne = flagWidth + 2 * PADDING;

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

    // Paint flag and country
    // Positions are mirrored for the blue player
    const int flagOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING : PADDING*2+flagHeight);
    const int countryOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING*2+flagHeight : PADDING);

    QRect flagRect(PADDING, flagOffset, flagWidth, flagHeight);
    QRect countryRect(PADDING, countryOffset, flagWidth, flagHeight);

    // Paint flag
    std::optional<PlayerCountry> country = player.getCountry();

    if (country.has_value()) {
        FlagImage &flag = mFlags[static_cast<size_t>(playerIndex)];
        flag.update(country);

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::NoBrush);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.drawImage(flagRect, *(flag.image));
        painter.setRenderHint(QPainter::Antialiasing, false);
    }

    auto font = mFont;

    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.setPen(COLOR_SCOREBOARD_BLACK);
    else
        painter.setPen(COLOR_SCOREBOARD_WHITE);

    // Paint country name
    if (country.has_value()) {
        QString countryText = QString::fromStdString(country->countryCode());
        font.setPixelSize(flagHeight*3/5);
        painter.setFont(font);

        painter.drawText(countryRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignTop : Qt::AlignBottom) | Qt::AlignHCenter, countryText);
    }

    // Paint name
    QRect nameRect(columnOne, PADDING, rect.width() - PADDING - columnOne, rect.height() - 2 * PADDING);
    font.setPixelSize(rect.height()/4);
    painter.setFont(font);
    QString nameText = QString::fromStdString(player.getLastName()) + QString(" ") + QString::fromStdString(player.getFirstName()).at(0) + QString(".");
    painter.drawText(nameRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignTop : Qt::AlignBottom) | Qt::AlignLeft, nameText);

    painter.restore();
}

void InternationalScoreboardPainter::paintIntroductionLower(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    const int flagHeight = (rect.height() - PADDING * 3) / 2;
    // const int flagWidth = flagHeight * 4 / 3;

    // const int columnOne = flagWidth + 2 * PADDING;
    // const int columnThree = columnOne + (rect.width() - columnOne) * 3 / 4;

    painter.save();
    painter.translate(rect.x(), rect.y());

    // Paint background
    QRect boundingRect(0, 0, rect.width(), rect.height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_SCOREBOARD_BLACK);
    painter.drawRect(boundingRect);

    // Paint title and category
    QRect titleRect(PADDING, PADDING, rect.width() - PADDING*2, flagHeight);
    QRect categoryRect(PADDING, PADDING*2+flagHeight, rect.width() - PADDING*2, flagHeight);

    auto font = mFont;
    font.setPixelSize(flagHeight*2/3);
    painter.setFont(font);
    painter.setPen(COLOR_SCOREBOARD_WHITE);

    painter.drawText(titleRect, Qt::AlignBottom | Qt::AlignLeft, QString::fromStdString(params.match.getTitle()));
    painter.drawText(categoryRect, Qt::AlignTop | Qt::AlignLeft, QString::fromStdString(params.category.getName()));

    painter.restore();
}

void InternationalScoreboardPainter::paintNormalPlayer(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const PlayerStore &player = (playerIndex == MatchStore::PlayerIndex::WHITE ? params.whitePlayer : params.bluePlayer);
    const int flagHeight = (rect.height() - PADDING * 3) / 2;
    const int flagWidth = flagHeight * 4 / 3;

    const int columnOne = flagWidth + 2 * PADDING;
    const int columnThree = columnOne + (rect.width() - columnOne) * 3 / 4;

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

    // Paint flag and country
    // Positions are mirrored for the blue player
    const int flagOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING : PADDING*2+flagHeight);
    const int countryOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING*2+flagHeight : PADDING);

    QRect flagRect(PADDING, flagOffset, flagWidth, flagHeight);
    QRect countryRect(PADDING, countryOffset, flagWidth, flagHeight);

    // Paint flag
    std::optional<PlayerCountry> country = player.getCountry();

    if (country.has_value()) {
        FlagImage &flag = mFlags[static_cast<size_t>(playerIndex)];
        flag.update(country);

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::NoBrush);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.drawImage(flagRect, *(flag.image));
        painter.setRenderHint(QPainter::Antialiasing, false);
    }

    auto font = mFont;

    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.setPen(COLOR_SCOREBOARD_BLACK);
    else
        painter.setPen(COLOR_SCOREBOARD_WHITE);

    // Paint country name
    if (country.has_value()) {
        QString countryText = QString::fromStdString(country->countryCode());
        font.setPixelSize(flagHeight*3/5);
        painter.setFont(font);

        painter.drawText(countryRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignTop : Qt::AlignBottom) | Qt::AlignHCenter, countryText);
    }

    // Paint name
    const int nameHeight = (rect.height() - PADDING * 3) / 3;
    const int scoreHeight = rect.height() - PADDING * 3 - nameHeight;

    const int nameOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING : PADDING*2+scoreHeight);
    const int scoreOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING*2+nameHeight : PADDING);
    QRect nameRect(columnOne, nameOffset, rect.width() - columnOne - PADDING, nameHeight);

    font.setPixelSize(nameHeight*4/5);
    painter.setFont(font);
    QString nameText = QString::fromStdString(player.getLastName()) + QString(" ") + QString::fromStdString(player.getFirstName()).at(0) + QString(".");
    painter.drawText(nameRect, Qt::AlignTop | Qt::AlignLeft, nameText);

    // Score
    QRect scoreRect(columnOne, scoreOffset, columnThree - columnOne - PADDING, scoreHeight);

    font.setPixelSize(scoreHeight*4/5);
    painter.setFont(font);

    const auto &score = params.match.getScore(playerIndex);
    const auto &otherScore = params.match.getScore(playerIndex == MatchStore::PlayerIndex::WHITE ? MatchStore::PlayerIndex::BLUE : MatchStore::PlayerIndex::WHITE);
    if (!score.hansokuMake && (score.ippon || otherScore.hansokuMake))
        painter.drawText(scoreRect, Qt::AlignBottom | Qt::AlignRight, "IPPON");
    else
        painter.drawText(scoreRect, Qt::AlignBottom | Qt::AlignRight, QString::number(score.wazari));

    // Penalties
    const int penaltyHeight = scoreHeight/2;
    const int penaltyWidth = penaltyHeight*3/4;
    QRect firstPenaltyRect(columnThree, scoreOffset+(scoreHeight-penaltyHeight)/2, penaltyWidth, penaltyHeight);
    QRect secondPenaltyRect(columnThree + PADDING + penaltyWidth, scoreOffset+(scoreHeight-penaltyHeight)/2, penaltyWidth, penaltyHeight);

    if (score.hansokuMake) {
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

void InternationalScoreboardPainter::paintNormalLower(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    const int flagHeight = (rect.height() - PADDING * 3) / 2;
    const int flagWidth = flagHeight * 4 / 3;

    const int columnOne = flagWidth + 2 * PADDING;
    const int columnThree = columnOne + (rect.width() - columnOne) * 3 / 4;
    const int columnTwo = columnOne + (columnThree - columnOne) / 3;

    const auto &ruleset = params.category.getRuleset();

    painter.save();
    painter.translate(rect.x(), rect.y());

    // Paint background
    QRect boundingRect(0, 0, rect.width(), rect.height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_SCOREBOARD_BLACK);
    painter.drawRect(boundingRect);

    // Paint title and category
    QRect titleRect(PADDING, PADDING, columnTwo-PADDING, flagHeight);
    QRect categoryRect(PADDING, PADDING*2+flagHeight, columnTwo-PADDING, flagHeight);

    auto font = mFont;
    font.setPixelSize(flagHeight*1/2);
    painter.setFont(font);
    painter.setPen(COLOR_SCOREBOARD_WHITE);

    painter.drawText(titleRect, Qt::AlignBottom | Qt::AlignLeft, QString::fromStdString(params.match.getTitle()));
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

void FlagImage::update(std::optional<PlayerCountry> country) {
    if (country == this->country)
        return;

    this->country = country;

    if (!country.has_value()) {
        this->image = std::nullopt;
        return;
    }

    QString countryCode = QString::fromStdString(country->countryCode());
    QString filename = QString(DATA_DIR) + QString("/flags/%1.svg").arg(countryCode.toLower());
    this->image = QImage(filename);
}

void InternationalScoreboardPainter::resizeEvent(const QRect &rect) {

}

