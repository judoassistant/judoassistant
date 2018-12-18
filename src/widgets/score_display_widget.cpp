#include <QPainter>

#include "widgets/score_display_widget.hpp"
#include "widgets/colors.hpp"
#include "stores/qtournament_store.hpp"
#include "stores/category_store.hpp"
#include "stores/player_store.hpp"

ScoreDisplayWidget::ScoreDisplayWidget(const StoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
    , mState(ScoreDisplayState::INTRODUCTION)
    , mFont("Noto Sans")
{
    mIntroTimer.setSingleShot(true);
    mWinnerTimer.setSingleShot(true);

    mFont.setBold(true);
    mFont.setCapitalization(QFont::AllUppercase);

    connect(&mIntroTimer, &QTimer::timeout, [this](){ setState(ScoreDisplayState::NORMAL); });
    connect(&mWinnerTimer, &QTimer::timeout, [this](){ setState(ScoreDisplayState::WINNER); });
}

void ScoreDisplayWidget::paintNullMatch(QPainter &painter) {
    QRect rect(0, 0, width(), height());

    // Draw Background
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_0);

    painter.drawRect(rect);
}

void ScoreDisplayWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setFont(mFont);
    painter.setRenderHint(QPainter::Antialiasing, false);

    if (!mCombinedId) {
        paintNullMatch(painter);
        return;
    }

    const auto &tournament = mStoreManager.getTournament();
    const auto &category = tournament.getCategory(mCombinedId->first);
    const auto &match = category.getMatch(mCombinedId->second);

    if (!match.getWhitePlayer() || !match.getBluePlayer()) {
        log_warning().msg("The current match does not have 2 players");
        paintNullMatch(painter);
        return;
    }

    const auto &whitePlayer = tournament.getPlayer(*(match.getWhitePlayer()));
    const auto &bluePlayer = tournament.getPlayer(*(match.getBluePlayer()));

    QRect upperRect(0,0,width(), height()/3);
    QRect middleRect(0,height()/3,width(), height()/3);
    QRect lowerRect(0,2*(height()/3),width(), height() - 2*(height()/3));


    if (mState == ScoreDisplayState::INTRODUCTION) {
        log_debug().msg("Painting intro screen");
        paintPlayerIntroduction(upperRect, MatchStore::PlayerIndex::WHITE, painter, match, whitePlayer);
        paintPlayerIntroduction(middleRect, MatchStore::PlayerIndex::BLUE, painter, match, bluePlayer);
        paintLowerIntroduction(lowerRect, painter, category, match);
        return;
    }
    else if (mState == ScoreDisplayState::NORMAL) {
        log_debug().msg("Painting normal screen");
        paintPlayerNormal(upperRect, MatchStore::PlayerIndex::WHITE, painter, match, whitePlayer);
        paintPlayerNormal(middleRect, MatchStore::PlayerIndex::BLUE, painter, match, bluePlayer);
        paintLowerNormal(lowerRect, painter, category, match);
    }
    else {
        assert(mState == ScoreDisplayState::WINNER);
        log_debug().msg("Painting winner screen");
        // TODO: Paint finished screen
        paintPlayerNormal(upperRect, MatchStore::PlayerIndex::WHITE, painter, match, whitePlayer);
        paintPlayerNormal(middleRect, MatchStore::PlayerIndex::BLUE, painter, match, bluePlayer);
        paintLowerNormal(lowerRect, painter, category, match);
    }

    // paintPlayerIntroduction(upperRect, MatchStore::PlayerIndex::WHITE,
    // paintFirstCompetitor(painter);
    // paintSecondCompetitor(painter);
    // paintLowerSection(painter);
}

void ScoreDisplayWidget::setMatch(std::optional<std::pair<CategoryId, MatchId>> combinedId, bool showIntro) {
    mCombinedId = combinedId;
    if (showIntro) {
        mState = ScoreDisplayState::INTRODUCTION;
        mIntroTimer.start(INTRO_INTERVAL);
    }
    else {
        mState = ScoreDisplayState::NORMAL;
    }

    update(0, 0, width(), height());
}

void ScoreDisplayWidget::paintPlayerNormal(QRect rect, MatchStore::PlayerIndex playerIndex, QPainter &painter, const MatchStore &match, const PlayerStore &player) {
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
        painter.setBrush(Qt::white);
    else
        painter.setBrush(COLOR_10);
    painter.drawRect(boundingRect);

    // Paint flag and country
    // Positions are mirrored for the blue player
    const int flagOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING : PADDING*2+flagHeight);
    const int countryOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING*2+flagHeight : PADDING);

    QRect flagRect(PADDING, flagOffset, flagWidth, flagHeight);
    QRect countryRect(PADDING, countryOffset, flagWidth, flagHeight);

    // Paint flag
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_14);
    painter.drawRect(flagRect);

    // Paint country name
    auto font = mFont;
    font.setPixelSize(flagHeight*3/5);
    painter.setFont(font);

    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.setPen(COLOR_0);
    else
        painter.setPen(Qt::white);
    painter.drawText(countryRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignTop : Qt::AlignBottom) | Qt::AlignHCenter, "DEN");

    // Paint name
    const int nameHeight = (rect.height() - PADDING * 3) / 3;
    const int scoreHeight = rect.height() - PADDING * 3 - nameHeight;

    const int nameOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING : PADDING*2+scoreHeight);
    const int scoreOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING*2+nameHeight : PADDING);
    QRect nameRect(columnOne, nameOffset, rect.width() - columnOne - PADDING, nameHeight);

    font.setPixelSize(nameHeight*4/5);
    painter.setFont(font);
    QString nameText = QString::fromStdString(player.getLastName()) + QString(" ") + QString::fromStdString(player.getFirstName()).front() + QString(".");
    painter.drawText(nameRect, Qt::AlignTop | Qt::AlignLeft, nameText);

    // Score
    QRect scoreRect(columnOne, scoreOffset, columnThree - columnOne - PADDING, scoreHeight);

    font.setPixelSize(scoreHeight*4/5);
    painter.setFont(font);
    painter.setBrush(COLOR_11);

    const auto &score = match.getScore(playerIndex);
    if (score.ippon > 0)
        painter.drawText(scoreRect, Qt::AlignBottom | Qt::AlignHCenter, "IPPON");
    else
        painter.drawText(scoreRect, Qt::AlignBottom | Qt::AlignRight, QString::number(score.wazari));

    // Penalties
    const int penaltyHeight = scoreHeight/2;
    const int penaltyWidth = penaltyHeight*3/4;
    QRect firstPenaltyRect(columnThree, scoreOffset+(scoreHeight-penaltyHeight)/2, penaltyWidth, penaltyHeight);
    QRect secondPenaltyRect(columnThree + PADDING + penaltyWidth, scoreOffset+(scoreHeight-penaltyHeight)/2, penaltyWidth, penaltyHeight);

    if (score.hansokuMake > 0) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(COLOR_11);
        painter.drawRect(firstPenaltyRect);

    }
    else {
        painter.setPen(Qt::NoPen);
        painter.setBrush(COLOR_13);
        if (score.shido > 0)
            painter.drawRect(firstPenaltyRect);
        if (score.shido > 1)
            painter.drawRect(secondPenaltyRect);

    }

    painter.restore();
}

void ScoreDisplayWidget::paintLowerNormal(QRect rect, QPainter &painter, const CategoryStore &category, const MatchStore &match) {
    const int flagHeight = (rect.height() - PADDING * 3) / 2;
    const int flagWidth = flagHeight * 4 / 3;

    const int columnOne = flagWidth + 2 * PADDING;
    const int columnThree = columnOne + (rect.width() - columnOne) * 3 / 4;
    const int columnTwo = columnOne + (columnThree - columnOne) / 3;

    painter.save();
    painter.translate(rect.x(), rect.y());

    // Paint background
    QRect boundingRect(0, 0, rect.width(), rect.height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_0);
    painter.drawRect(boundingRect);

    // Paint title and category
    QRect titleRect(PADDING, PADDING, columnTwo-PADDING, flagHeight);
    QRect categoryRect(PADDING, PADDING*2+flagHeight, columnTwo-PADDING, flagHeight);

    auto font = mFont;
    font.setPixelSize(flagHeight*1/2);
    painter.setFont(font);
    painter.setPen(COLOR_6);

    painter.drawText(titleRect, Qt::AlignBottom | Qt::AlignLeft, QString::fromStdString(match.getTitle()));
    painter.drawText(categoryRect, Qt::AlignTop | Qt::AlignLeft, QString::fromStdString(category.getName()));

    // Paint time left
    QRect timeRect(columnTwo, PADDING, columnThree-columnTwo-PADDING, rect.height()-PADDING*2);

    font.setPixelSize(rect.height()*6/8);
    painter.setFont(font);

    painter.setPen(COLOR_14);
    painter.drawText(timeRect, Qt::AlignVCenter | Qt::AlignRight, "3:56");

    // Paint golden score indicator
    if (match.isGoldenScore()) {
        QRect goldenScoreRect(columnThree, PADDING, columnThree-columnTwo, rect.height()-PADDING*2);

        font.setPixelSize(rect.height()*1/4);
        painter.setFont(font);

        painter.setPen(COLOR_13);
        painter.drawText(goldenScoreRect, Qt::AlignVCenter | Qt::AlignLeft, "GS");
    }

    // TODO: Make font sizing more robust
    painter.restore();
}

void ScoreDisplayWidget::paintPlayerIntroduction(QRect rect, MatchStore::PlayerIndex playerIndex, QPainter &painter, const MatchStore &match, const PlayerStore &player) {
    const int flagHeight = (rect.height() - PADDING * 3) / 2;
    const int flagWidth = flagHeight * 4 / 3;

    const int columnOne = flagWidth + 2 * PADDING;

    painter.save();
    painter.translate(rect.x(), rect.y());

    // Paint background
    QRect boundingRect(0, 0, rect.width(), rect.height());
    painter.setPen(Qt::NoPen);
    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.setBrush(Qt::white);
    else
        painter.setBrush(COLOR_10);
    painter.drawRect(boundingRect);

    // Paint flag and country
    // Positions are mirrored for the blue player
    const int flagOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING : PADDING*2+flagHeight);
    const int countryOffset = (playerIndex == MatchStore::PlayerIndex::WHITE ? PADDING*2+flagHeight : PADDING);

    QRect flagRect(PADDING, flagOffset, flagWidth, flagHeight);
    QRect countryRect(PADDING, countryOffset, flagWidth, flagHeight);

    // Paint flag
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_14);
    painter.drawRect(flagRect);

    // Paint country name
    auto font = mFont;
    font.setPixelSize(flagHeight*3/5);
    painter.setFont(font);

    if (playerIndex == MatchStore::PlayerIndex::WHITE)
        painter.setPen(COLOR_0);
    else
        painter.setPen(Qt::white);
    painter.drawText(countryRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignTop : Qt::AlignBottom) | Qt::AlignHCenter, "DEN");

    // Paint name
    QRect nameRect(columnOne, PADDING, rect.width() - PADDING - columnOne, rect.height() - 2 * PADDING);
    font.setPixelSize(rect.height()/4);
    painter.setFont(font);
    QString nameText = QString::fromStdString(player.getLastName()) + QString(" ") + QString::fromStdString(player.getFirstName()).front() + QString(".");
    painter.drawText(nameRect, (playerIndex == MatchStore::PlayerIndex::WHITE ? Qt::AlignTop : Qt::AlignBottom) | Qt::AlignLeft, nameText);

    painter.restore();
}

void ScoreDisplayWidget::paintLowerIntroduction(QRect rect, QPainter &painter, const CategoryStore &category, const MatchStore &match) {
    // TODO: Refactor code to reduce redundancy
    const int flagHeight = (rect.height() - PADDING * 3) / 2;
    const int flagWidth = flagHeight * 4 / 3;

    const int columnOne = flagWidth + 2 * PADDING;
    const int columnThree = columnOne + (rect.width() - columnOne) * 3 / 4;

    painter.save();
    painter.translate(rect.x(), rect.y());

    // Paint background
    QRect boundingRect(0, 0, rect.width(), rect.height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_0);
    painter.drawRect(boundingRect);

    // Paint title and category
    QRect titleRect(PADDING, PADDING, rect.width() - PADDING*2, flagHeight);
    QRect categoryRect(PADDING, PADDING*2+flagHeight, rect.width() - PADDING*2, flagHeight);

    auto font = mFont;
    font.setPixelSize(flagHeight*2/3);
    painter.setFont(font);
    painter.setPen(COLOR_6);

    painter.drawText(titleRect, Qt::AlignBottom | Qt::AlignLeft, QString::fromStdString(match.getTitle()));
    painter.drawText(categoryRect, Qt::AlignTop | Qt::AlignLeft, QString::fromStdString(category.getName()));

    // TODO: Show intro countdown on operator screen
    // TODO: Make font sizing more robust
    painter.restore();
}

void ScoreDisplayWidget::setState(ScoreDisplayState state) {
    mState = state;
    update(0, 0, width(), height());
}

