#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QShortcut>

#include "core/actions/award_hansoku_make_action.hpp"
#include "core/actions/award_ippon_action.hpp"
#include "core/actions/award_shido_action.hpp"
#include "core/actions/award_wazari_action.hpp"
#include "core/actions/cancel_hansoku_make_action.hpp"
#include "core/actions/cancel_ippon_action.hpp"
#include "core/actions/cancel_shido_action.hpp"
#include "core/actions/cancel_wazari_action.hpp"
#include "core/actions/pause_match_action.hpp"
#include "core/actions/resume_match_action.hpp"
#include "core/actions/start_osaekomi_action.hpp"
#include "core/actions/stop_osaekomi_action.hpp"
#include "core/log.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/player_store.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/score_operator_widget.hpp"

ScoreOperatorWidget::ScoreOperatorWidget(StoreManager &storeManager, QWidget *parent)
    : ScoreDisplayWidget(storeManager, parent)
    , mStoreManager(storeManager)
{
    connect(&mPausingTimer, &QTimer::timeout, this, &ScoreOperatorWidget::pausingTimerHit);
    mPausingTimer.start(PAUSING_TIMER_INTERVAL);

    auto *shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(shortcut, &QShortcut::activated, this, &ScoreOperatorWidget::durationShortcut);
}

void ScoreOperatorWidget::paintControls(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    painter.setPen(COLOR_SCOREBOARD_CONTROLS);
    painter.setBrush(Qt::NoBrush);

    const Ruleset &ruleset = params.category.getRuleset();

    // Pause/Resume control
    if (ruleset.canPause(params.match, params.masterTime) || ruleset.canResume(params.match, params.masterTime))
        painter.drawRect(mScoreboardPainter->getDurationRect());

    // White Player Controls
    painter.drawRect(mScoreboardPainter->getWhiteIpponRect());
    painter.drawRect(mScoreboardPainter->getWhiteWazariRect());
    painter.drawRect(mScoreboardPainter->getWhiteOsaekomiRect());
    painter.drawRect(mScoreboardPainter->getWhiteShidoRect());
    painter.drawRect(mScoreboardPainter->getWhiteHansokuRect());

    // Blue Player Controls
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

    ScoreboardPainterParams params{category, match, whitePlayer, bluePlayer, mStoreManager.masterTime(), false};
    bool shiftPressed = event->modifiers().testFlag(Qt::ShiftModifier);

    if (mScoreboardPainter->getDurationRect().contains(pos)) {
        durationClick(params);
    }
    else if (mScoreboardPainter->getWhiteOsaekomiRect().contains(pos)) {
        osaekomiClick(params, MatchStore::PlayerIndex::WHITE);
    }
    else if (mScoreboardPainter->getWhiteIpponRect().contains(pos)) {
        if (!shiftPressed)
            awardIppon(params, MatchStore::PlayerIndex::WHITE);
        else
            cancelIppon(params, MatchStore::PlayerIndex::WHITE);
    }
    else if (mScoreboardPainter->getWhiteWazariRect().contains(pos)) {
        if (!shiftPressed)
            awardWazari(params, MatchStore::PlayerIndex::WHITE);
        else
            cancelWazari(params, MatchStore::PlayerIndex::WHITE);
    }
    else if (mScoreboardPainter->getWhiteShidoRect().contains(pos)) {
        if (!shiftPressed)
            awardShido(params, MatchStore::PlayerIndex::WHITE);
        else
            cancelShido(params, MatchStore::PlayerIndex::WHITE);
    }
    else if (mScoreboardPainter->getWhiteHansokuRect().contains(pos)) {
        if (!shiftPressed)
            awardHansokuMake(params, MatchStore::PlayerIndex::WHITE);
        else
            cancelHansokuMake(params, MatchStore::PlayerIndex::WHITE);
    }
    else if (mScoreboardPainter->getBlueOsaekomiRect().contains(pos)) {
        osaekomiClick(params, MatchStore::PlayerIndex::BLUE);
    }
    else if (mScoreboardPainter->getBlueIpponRect().contains(pos)) {
        if (!shiftPressed)
            awardIppon(params, MatchStore::PlayerIndex::BLUE);
        else
            cancelIppon(params, MatchStore::PlayerIndex::BLUE);
    }
    else if (mScoreboardPainter->getBlueWazariRect().contains(pos)) {
        if (!shiftPressed)
            awardWazari(params, MatchStore::PlayerIndex::BLUE);
        else
            cancelWazari(params, MatchStore::PlayerIndex::BLUE);
    }
    else if (mScoreboardPainter->getBlueShidoRect().contains(pos)) {
        if (!shiftPressed)
            awardShido(params, MatchStore::PlayerIndex::BLUE);
        else
            cancelShido(params, MatchStore::PlayerIndex::BLUE);
    }
    else if (mScoreboardPainter->getBlueHansokuRect().contains(pos)) {
        if (!shiftPressed)
            awardHansokuMake(params, MatchStore::PlayerIndex::BLUE);
        else
            cancelHansokuMake(params, MatchStore::PlayerIndex::BLUE);
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

    ScoreboardPainterParams params{category, match, whitePlayer, bluePlayer, mStoreManager.masterTime(), false};

    if (mState == ScoreDisplayState::NORMAL)
        paintControls(painter, rect, params);
}

void ScoreOperatorWidget::durationClick(ScoreboardPainterParams &params) {
    const auto &ruleset = params.category.getRuleset();

    if (params.match.getStatus() == MatchStatus::UNPAUSED) {
        if (!ruleset.canPause(params.match, params.masterTime))
            return;
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));
    }
    else {
        if (!ruleset.canResume(params.match, params.masterTime))
            return;
        mStoreManager.dispatch(std::make_unique<ResumeMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));
    }
}

void ScoreOperatorWidget::osaekomiClick(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const auto &ruleset = params.category.getRuleset();

    auto osaekomi = params.match.getOsaekomi();
    if (!osaekomi.has_value() || osaekomi->first != playerIndex) {
        // Start osaekomi
        if (!ruleset.canStartOsaekomi(params.match, playerIndex))
            return;
        mStoreManager.dispatch(std::make_unique<StartOsaekomiAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));
    }
    else {
        // Stop osaekomi
        if (!ruleset.canStopOsaekomi(params.match, params.masterTime))
            return;
        mStoreManager.dispatch(std::make_unique<StopOsaekomiAction>(params.category.getId(), params.match.getId(), params.masterTime));
    }
}

void ScoreOperatorWidget::awardIppon(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardIppon(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<AwardIpponAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::awardWazari(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardWazari(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<AwardWazariAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::awardShido(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardShido(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<AwardShidoAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::awardHansokuMake(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardHansokuMake(params.match, playerIndex))
        return;

    auto reply = QMessageBox::question(this, tr("Would you like to award direct hansoku-make?"), tr("Are you sure you would like to award direct hansoku-make?"), QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel)
        return;

    mStoreManager.dispatch(std::make_unique<AwardHansokuMakeAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::pausingTimerHit() {
    auto optParams = getParams();
    if (!optParams)
        return;

    ScoreboardPainterParams params = *optParams;

    const auto &ruleset = params.category.getRuleset();
    if (ruleset.shouldAwardOsaekomiWazari(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<AwardWazariAction>(params.category.getId(), params.match.getId(), params.match.getOsaekomi()->first, params.masterTime, true));

    if (ruleset.shouldAwardOsaekomiIppon(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<AwardIpponAction>(params.category.getId(), params.match.getId(), params.match.getOsaekomi()->first, params.masterTime, true));

    if (ruleset.shouldStopOsaekomi(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<StopOsaekomiAction>(params.category.getId(),  params.match.getId(), params.masterTime));

    if (ruleset.shouldPause(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(),  params.match.getId(), params.masterTime));
}

std::optional<ScoreboardPainterParams> ScoreOperatorWidget::getParams() {
    if (!mCombinedId)
        return std::nullopt;

    const auto &tournament = mStoreManager.getTournament();

    if (!tournament.containsCategory(mCombinedId->first))
        return std::nullopt;

    const auto &category = tournament.getCategory(mCombinedId->first);

    if (!category.containsMatch(mCombinedId->second))
        return std::nullopt;

    const auto &match = category.getMatch(mCombinedId->second);

    if (!match.getWhitePlayer() || !match.getBluePlayer())
        return std::nullopt;

    const auto &whitePlayer = tournament.getPlayer(*(match.getWhitePlayer()));
    const auto &bluePlayer = tournament.getPlayer(*(match.getBluePlayer()));
    return ScoreboardPainterParams{category, match, whitePlayer, bluePlayer, mStoreManager.masterTime(), false};
}

void ScoreOperatorWidget::durationShortcut() {
    auto optParams = getParams();
    if (!optParams)
        return;

    ScoreboardPainterParams params = *optParams;

    durationClick(params);
}

void ScoreOperatorWidget::cancelIppon(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canCancelIppon(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<CancelIpponAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::cancelWazari(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canCancelWazari(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<CancelWazariAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::cancelShido(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canCancelShido(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<CancelShidoAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::cancelHansokuMake(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canCancelHansokuMake(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<CancelHansokuMakeAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));
}

