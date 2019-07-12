#include <QPainter>
#include <QMouseEvent>

#include "core/log.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/score_operator_widget.hpp"

ScoreOperatorWidget::ScoreOperatorWidget(const StoreManager &storeManager, QWidget *parent)
    : ScoreDisplayWidget(storeManager, parent)
{}

void ScoreOperatorWidget::paintControls(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    painter.setPen(COLOR_SCOREBOARD_CONTROLS);
    painter.setBrush(Qt::NoBrush);

    const Ruleset &ruleset = params.category.getRuleset();

    const auto masterTime = mStoreManager.masterTime();

    if (ruleset.canPause(params.match, masterTime) || ruleset.canResume(params.match, masterTime))
        painter.drawRect(mScoreboardPainter->getDurationRect());
    painter.drawRect(mScoreboardPainter->getWhiteIpponRect());
    painter.drawRect(mScoreboardPainter->getWhiteWazariRect());
    painter.drawRect(mScoreboardPainter->getWhiteOsaekomiRect());
    painter.drawRect(mScoreboardPainter->getWhiteShidoRect());
    painter.drawRect(mScoreboardPainter->getWhiteHansokuRect());

    painter.drawRect(mScoreboardPainter->getBlueIpponRect());
    painter.drawRect(mScoreboardPainter->getBlueWazariRect());
    painter.drawRect(mScoreboardPainter->getBlueOsaekomiRect());
    painter.drawRect(mScoreboardPainter->getBlueShidoRect());
    painter.drawRect(mScoreboardPainter->getBlueHansokuRect());
}

void ScoreOperatorWidget::mouseReleaseEvent(QMouseEvent *event) {
    const auto pos = event->pos();
    if (!mCombinedId)
        return;

    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCombinedId->first))
        return;

    const auto &category = tournament.getCategory(mCombinedId->first);
    if (!category.containsMatch(mCombinedId->second))
        return;

    const auto &match = category.getMatch(mCombinedId->second);
    if (!match.getWhitePlayer() || !match.getBluePlayer())
        return;

    const auto &whitePlayer = tournament.getPlayer(*(match.getWhitePlayer()));
    const auto &bluePlayer = tournament.getPlayer(*(match.getBluePlayer()));

    ScoreboardPainterParams params{category, match, whitePlayer, bluePlayer, mStoreManager.masterTime()};

    if (mScoreboardPainter->getDurationRect().contains(pos)) {
        log_debug().msg("Duration pressed");
    }
}

void ScoreOperatorWidget::paintEvent(QPaintEvent *event) {
    ScoreDisplayWidget::paintEvent(event);

    QPainter painter(this);

    QRect rect(0, 0, width(), height());

    if (!mCombinedId) {
        mScoreboardPainter->paintEmpty(painter, rect);
        return;
    }

    const auto &tournament = mStoreManager.getTournament();

    if (!tournament.containsCategory(mCombinedId->first))
        return;

    const auto &category = tournament.getCategory(mCombinedId->first);

    if (!category.containsMatch(mCombinedId->second))
        return;

    const auto &match = category.getMatch(mCombinedId->second);

    if (!match.getWhitePlayer() || !match.getBluePlayer())
        return;

    const auto &whitePlayer = tournament.getPlayer(*(match.getWhitePlayer()));
    const auto &bluePlayer = tournament.getPlayer(*(match.getBluePlayer()));

    ScoreboardPainterParams params{category, match, whitePlayer, bluePlayer, mStoreManager.masterTime()};

    if (mState == ScoreDisplayState::NORMAL)
        paintControls(painter, rect, params);
}

