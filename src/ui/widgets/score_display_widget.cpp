#include <QPainter>
#include <QMouseEvent>

#include "core/log.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/score_display_widget.hpp"
#include "ui/widgets/scoreboard_painters/international_scoreboard_painter.hpp"
#include "ui/widgets/scoreboard_painters/national_scoreboard_painter.hpp"

ScoreDisplayWidget::ScoreDisplayWidget(const StoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mState(ScoreDisplayState::INTRODUCTION)
    , mStoreManager(storeManager)
{
    loadPainter();

    mIntroTimer.setSingleShot(true);
    mWinnerTimer.setSingleShot(true);
    mDurationTimer.start(DURATION_INTERVAL);

    connect(&mIntroTimer, &QTimer::timeout, [this](){ setState(ScoreDisplayState::NORMAL); });
    connect(&mWinnerTimer, &QTimer::timeout, [this](){ setState(ScoreDisplayState::WINNER); });
    connect(&mDurationTimer, &QTimer::timeout, this, &ScoreDisplayWidget::durationTimerHit);

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &ScoreDisplayWidget::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &ScoreDisplayWidget::endResetTournament);
}

void ScoreDisplayWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    QRect rect(0, 0, width(), height());

    if (!mCombinedId) {
        mScoreboardPainter->paintEmpty(painter, rect);
        return;
    }

    const auto &tournament = mStoreManager.getTournament();

    if (!tournament.containsCategory(mCombinedId->getCategoryId())) {
        mScoreboardPainter->paintEmpty(painter, rect);
        return;
    }

    const auto &category = tournament.getCategory(mCombinedId->getCategoryId());

    if (!category.containsMatch(mCombinedId->getMatchId())) {
        mScoreboardPainter->paintEmpty(painter, rect);
        return;
    }
    const auto &match = category.getMatch(mCombinedId->getMatchId());

    if (!match.getWhitePlayer() || !match.getBluePlayer()) {
        mScoreboardPainter->paintEmpty(painter, rect);
        return;
    }

    const auto &whitePlayer = tournament.getPlayer(*(match.getWhitePlayer()));
    const auto &bluePlayer = tournament.getPlayer(*(match.getBluePlayer()));

    std::chrono::milliseconds masterTime = mStoreManager.masterTime();
    bool blink = false;
    if (match.getStatus() == MatchStatus::UNPAUSED) {
        std::chrono::milliseconds diff = match.getResumeTime() - masterTime;
        blink = (diff.count() / (100*5)) % 2 == 0;
    }
    ScoreboardPainterParams params{category, match, whitePlayer, bluePlayer, masterTime, blink};

    if (mState == ScoreDisplayState::INTRODUCTION) {
        mScoreboardPainter->paintIntroduction(painter, rect, params);
    }
    else if (mState == ScoreDisplayState::NORMAL) {
        mScoreboardPainter->paintNormal(painter, rect, params);
    }
    else {
        assert(mState == ScoreDisplayState::WINNER);
        mScoreboardPainter->paintWinner(painter, rect, params);
    }
}

void ScoreDisplayWidget::setMatch(std::optional<CombinedId> combinedId, bool showIntro) {
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

void ScoreDisplayWidget::setState(ScoreDisplayState state) {
    mState = state;
    update(0, 0, width(), height());
}

void ScoreDisplayWidget::beginResetTournament() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
        mConnections.pop();
    }
}

void ScoreDisplayWidget::endResetTournament() {
    const QTournamentStore & tournament = mStoreManager.getTournament();
    mConnections.push(connect(&tournament, &QTournamentStore::matchesChanged, this, &ScoreDisplayWidget::changeMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::playersChanged, this, &ScoreDisplayWidget::changePlayers));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesReset, this, &ScoreDisplayWidget::resetMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::categoriesChanged, this, &ScoreDisplayWidget::changeCategories));
    mConnections.push(connect(&tournament, &QTournamentStore::preferencesChanged, this, &ScoreDisplayWidget::loadPainter));

    loadPainter();
}

void ScoreDisplayWidget::changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {
    if (!mCombinedId) return;
    if (mCombinedId->getCategoryId() != categoryId) return;

    for (auto matchId : matchIds) {
        if (mCombinedId->getMatchId() == matchId) {
            if (mState == ScoreDisplayState::INTRODUCTION)
                mState = ScoreDisplayState::NORMAL;
            update(0, 0, width(), height());
            return;
        }
    }
}

void ScoreDisplayWidget::changePlayers(std::vector<PlayerId> playerIds) {
    if (!mCombinedId)
        return;

    const auto &tournament = mStoreManager.getTournament();

    if (!tournament.containsCategory(mCombinedId->getCategoryId()))
        return;

    const auto &category = tournament.getCategory(mCombinedId->getCategoryId());

    if (!category.containsMatch(mCombinedId->getMatchId()))
        return;
    const auto &match = category.getMatch(mCombinedId->getMatchId());

    for (auto playerId : playerIds) {
        if (match.getWhitePlayer() == playerId || match.getBluePlayer() == playerId) {
            update(0, 0, width(), height());
            return;
        }
    }
}

void ScoreDisplayWidget::resetMatches(const std::vector<CategoryId> &categoryIds) {
    if (!mCombinedId)
        return;

    for (auto categoryId : categoryIds) {
        if (mCombinedId->getCategoryId() == categoryId) {
            update(0, 0, width(), height());
            return;
        }
    }
}

void ScoreDisplayWidget::changeCategories(std::vector<CategoryId> categoryIds) {
    for (auto categoryId : categoryIds) {
        if (mCombinedId && mCombinedId->getCategoryId() == categoryId) {
            update(0, 0, width(), height());
            return;
        }
    }
}

void ScoreDisplayWidget::durationTimerHit() {
    if (!mCombinedId)
        return;
    if (mState != ScoreDisplayState::NORMAL)
        return;
    // QRect lowerRect(0,2*(height()/3),width(), height() - 2*(height()/3));
    // update(lowerRect);
    update(rect());
}

void ScoreDisplayWidget::loadPainter() {
    const auto &preferences = mStoreManager.getTournament().getPreferences();
    auto style = preferences.getScoreboardStyle();

    if (mScoreboardPainter != nullptr && mScoreboardStyle == style)
        return;

    mScoreboardStyle = style;
    if (style == ScoreboardStylePreference::INTERNATIONAL)
        mScoreboardPainter = std::make_unique<InternationalScoreboardPainter>();
    else if (style == ScoreboardStylePreference::NATIONAL)
        mScoreboardPainter = std::make_unique<NationalScoreboardPainter>();

    mScoreboardPainter->resizeEvent(rect());

    update(0, 0, width(), height());
}

void ScoreDisplayWidget::resizeEvent(QResizeEvent *event) {
    mScoreboardPainter->resizeEvent(rect());
}

