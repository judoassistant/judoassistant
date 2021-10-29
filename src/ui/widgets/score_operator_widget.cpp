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
    , mFont("Noto Sans Mono")
{
    connect(&mPausingTimer, &QTimer::timeout, this, &ScoreOperatorWidget::pausingTimerHit);
    mPausingTimer.start(PAUSING_TIMER_INTERVAL);

    // Shortcuts
    auto *timerShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(timerShortcut, &QShortcut::activated, this, &ScoreOperatorWidget::durationShortcut);

    auto *whiteOsaeokomiShortcut = new QShortcut(QKeySequence(Qt::Key_1), this);
    connect(whiteOsaeokomiShortcut, &QShortcut::activated, this, &ScoreOperatorWidget::whiteOsaekomiShortcut);

    auto *blueOsaekomiShortcut = new QShortcut(QKeySequence(Qt::Key_2), this);
    connect(blueOsaekomiShortcut, &QShortcut::activated, this, &ScoreOperatorWidget::blueOsaekomiShortcut);

    // setup font
    mFont.setBold(false);
    mFont.setCapitalization(QFont::AllUppercase);
}

void ScoreOperatorWidget::paintControls(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    painter.setPen(COLOR_SCOREBOARD_CONTROLS);
    painter.setBrush(Qt::NoBrush);

    const Ruleset &ruleset = params.category.getRuleset();

    // Set font size for controls
    auto font = mFont;
    font.setPixelSize(mScoreboardPainter->getWhiteIpponRect().height() * 0.2);
    painter.setFont(font);

    // Pause/Resume control
    if (ruleset.canPause(params.match, params.masterTime) || ruleset.canResume(params.match, params.masterTime)) {
        painter.drawRect(mScoreboardPainter->getDurationRect());

        auto label = (params.match.getStatus() == MatchStatus::UNPAUSED ? tr("Stop Timer") : tr("Start Timer"));
        painter.drawText(mScoreboardPainter->getDurationRect(), Qt::AlignTop | Qt::AlignHCenter, label);
    }

    // White Player Score Controls
    painter.drawRect(mScoreboardPainter->getWhiteIpponRect());
    painter.drawText(mScoreboardPainter->getWhiteIpponRect(), Qt::AlignTop | Qt::AlignHCenter, tr("Ippon"));

    painter.drawRect(mScoreboardPainter->getWhiteWazariRect());
    painter.drawText(mScoreboardPainter->getWhiteWazariRect(), Qt::AlignTop | Qt::AlignHCenter, tr("Wazari"));

    // Blue Player Score Controls
    painter.drawRect(mScoreboardPainter->getBlueIpponRect());
    painter.drawText(mScoreboardPainter->getBlueIpponRect(), Qt::AlignTop | Qt::AlignHCenter, tr("Ippon"));

    painter.drawRect(mScoreboardPainter->getBlueWazariRect());
    painter.drawText(mScoreboardPainter->getBlueWazariRect(), Qt::AlignTop | Qt::AlignHCenter, tr("Wazari"));

    // Osaekomi Controls
    font.setPixelSize(mScoreboardPainter->getWhiteOsaekomiRect().height() * 0.2);
    painter.setFont(font);

    painter.drawRect(mScoreboardPainter->getWhiteOsaekomiRect());
    painter.drawText(mScoreboardPainter->getWhiteOsaekomiRect(), Qt::AlignTop | Qt::AlignHCenter, tr("White Player Osaekomi"));

    painter.drawRect(mScoreboardPainter->getBlueOsaekomiRect());
    painter.drawText(mScoreboardPainter->getBlueOsaekomiRect(), Qt::AlignTop | Qt::AlignHCenter, tr("Blue Player Osaekomi"));

    // Set font size for shido/hansoku controls
    font.setPixelSize(mScoreboardPainter->getWhiteShidoRect().height() * 0.15);
    painter.setFont(font);

    // White shido/hansoku controls
    painter.drawRect(mScoreboardPainter->getWhiteShidoRect());
    painter.drawText(mScoreboardPainter->getWhiteShidoRect(), Qt::AlignTop | Qt::AlignHCenter, tr("Shido"));

    painter.drawRect(mScoreboardPainter->getWhiteHansokuRect());
    painter.drawText(mScoreboardPainter->getWhiteHansokuRect(), Qt::AlignTop | Qt::AlignHCenter, tr("Hansoku"));

    // Blue shido/hansoku controls
    painter.drawRect(mScoreboardPainter->getBlueShidoRect());
    painter.drawText(mScoreboardPainter->getBlueShidoRect(), Qt::AlignTop | Qt::AlignHCenter, tr("Shido"));

    painter.drawRect(mScoreboardPainter->getBlueHansokuRect());
    painter.drawText(mScoreboardPainter->getBlueHansokuRect(), Qt::AlignTop | Qt::AlignHCenter, tr("Hansoku"));
}

void ScoreOperatorWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton)
        return;

    const auto pos = event->pos();
    if (!mCombinedId)
        return;

    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCombinedId->getCategoryId()))
        return;

    const auto &category = tournament.getCategory(mCombinedId->getCategoryId());
    if (!category.containsMatch(mCombinedId->getMatchId()))
        return;

    const auto &match = category.getMatch(mCombinedId->getMatchId());
    if (!match.getWhitePlayer() || !match.getBluePlayer())
        return;

    const auto &whitePlayer = tournament.getPlayer(*(match.getWhitePlayer()));
    const auto &bluePlayer = tournament.getPlayer(*(match.getBluePlayer()));

    ScoreboardPainterParams params{category, match, whitePlayer, bluePlayer, mStoreManager.masterTime(), false};
    bool cancelModifier = event->modifiers().testFlag(Qt::ShiftModifier) || event->button() == Qt::RightButton;

    if (mScoreboardPainter->getDurationRect().contains(pos)) {
        durationClick(params);
    }
    else if (mScoreboardPainter->getWhiteOsaekomiRect().contains(pos)) {
        osaekomiClick(params, MatchStore::PlayerIndex::WHITE);
    }
    else if (mScoreboardPainter->getWhiteIpponRect().contains(pos)) {
        if (!cancelModifier)
            awardIppon(params, MatchStore::PlayerIndex::WHITE);
        else
            cancelIppon(params, MatchStore::PlayerIndex::WHITE);
    }
    else if (mScoreboardPainter->getWhiteWazariRect().contains(pos)) {
        if (!cancelModifier)
            awardWazari(params, MatchStore::PlayerIndex::WHITE);
        else
            cancelWazari(params, MatchStore::PlayerIndex::WHITE);
    }
    else if (mScoreboardPainter->getWhiteShidoRect().contains(pos)) {
        if (!cancelModifier)
            awardShido(params, MatchStore::PlayerIndex::WHITE);
        else
            cancelShido(params, MatchStore::PlayerIndex::WHITE);
    }
    else if (mScoreboardPainter->getWhiteHansokuRect().contains(pos)) {
        if (!cancelModifier)
            awardHansokuMake(params, MatchStore::PlayerIndex::WHITE);
        else
            cancelHansokuMake(params, MatchStore::PlayerIndex::WHITE);
    }
    else if (mScoreboardPainter->getBlueOsaekomiRect().contains(pos)) {
        osaekomiClick(params, MatchStore::PlayerIndex::BLUE);
    }
    else if (mScoreboardPainter->getBlueIpponRect().contains(pos)) {
        if (!cancelModifier)
            awardIppon(params, MatchStore::PlayerIndex::BLUE);
        else
            cancelIppon(params, MatchStore::PlayerIndex::BLUE);
    }
    else if (mScoreboardPainter->getBlueWazariRect().contains(pos)) {
        if (!cancelModifier)
            awardWazari(params, MatchStore::PlayerIndex::BLUE);
        else
            cancelWazari(params, MatchStore::PlayerIndex::BLUE);
    }
    else if (mScoreboardPainter->getBlueShidoRect().contains(pos)) {
        if (!cancelModifier)
            awardShido(params, MatchStore::PlayerIndex::BLUE);
        else
            cancelShido(params, MatchStore::PlayerIndex::BLUE);
    }
    else if (mScoreboardPainter->getBlueHansokuRect().contains(pos)) {
        if (!cancelModifier)
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

    if (!tournament.containsCategory(mCombinedId->getCategoryId()))
        return;

    const auto &category = tournament.getCategory(mCombinedId->getCategoryId());

    if (!category.containsMatch(mCombinedId->getMatchId()))
        return;

    const auto &match = category.getMatch(mCombinedId->getMatchId());

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
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.match.getCombinedId(), params.masterTime));
    }
    else {
        if (!ruleset.canResume(params.match, params.masterTime))
            return;
        mStoreManager.dispatch(std::make_unique<ResumeMatchAction>(params.match.getCombinedId(), params.masterTime));
    }
}

void ScoreOperatorWidget::osaekomiClick(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const auto &ruleset = params.category.getRuleset();

    auto osaekomi = params.match.getOsaekomi();
    if (!osaekomi.has_value() || osaekomi->first != playerIndex) {
        // Start osaekomi
        if (!ruleset.canStartOsaekomi(params.match, playerIndex))
            return;
        mStoreManager.dispatch(std::make_unique<StartOsaekomiAction>(params.match.getCombinedId(), playerIndex, params.masterTime));
    }
    else {
        // Stop osaekomi
        if (!ruleset.canStopOsaekomi(params.match, params.masterTime))
            return;
        mStoreManager.dispatch(std::make_unique<StopOsaekomiAction>(params.match.getCombinedId(), params.masterTime));
    }
}

void ScoreOperatorWidget::awardIppon(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardIppon(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<AwardIpponAction>(params.match.getCombinedId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::awardWazari(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardWazari(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<AwardWazariAction>(params.match.getCombinedId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::awardShido(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardShido(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<AwardShidoAction>(params.match.getCombinedId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::awardHansokuMake(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardHansokuMake(params.match, playerIndex))
        return;

    auto reply = QMessageBox::question(this, tr("Would you like to award direct hansoku-make?"), tr("Are you sure you would like to award direct hansoku-make?"), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel)
        return;

    mStoreManager.dispatch(std::make_unique<AwardHansokuMakeAction>(params.match.getCombinedId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::pausingTimerHit() {
    auto optParams = getParams();
    if (!optParams)
        return;

    ScoreboardPainterParams params = *optParams;

    const auto &ruleset = params.category.getRuleset();
    if (ruleset.shouldAwardOsaekomiWazari(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<AwardWazariAction>(params.match.getCombinedId(), params.match.getOsaekomi()->first, params.masterTime, true));

    if (ruleset.shouldAwardOsaekomiIppon(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<AwardIpponAction>(params.match.getCombinedId(), params.match.getOsaekomi()->first, params.masterTime, true));

    if (ruleset.shouldStopOsaekomi(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<StopOsaekomiAction>(params.match.getCombinedId(), params.masterTime));

    if (ruleset.shouldPause(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.match.getCombinedId(), params.masterTime));
}

std::optional<ScoreboardPainterParams> ScoreOperatorWidget::getParams() {
    if (!mCombinedId)
        return std::nullopt;

    const auto &tournament = mStoreManager.getTournament();

    if (!tournament.containsCategory(mCombinedId->getCategoryId()))
        return std::nullopt;

    const auto &category = tournament.getCategory(mCombinedId->getCategoryId());

    if (!category.containsMatch(mCombinedId->getMatchId()))
        return std::nullopt;

    const auto &match = category.getMatch(mCombinedId->getMatchId());

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

void ScoreOperatorWidget::whiteOsaekomiShortcut() {
    auto optParams = getParams();
    if (!optParams)
        return;

    ScoreboardPainterParams params = *optParams;

    osaekomiClick(params, MatchStore::PlayerIndex::WHITE);
}

void ScoreOperatorWidget::blueOsaekomiShortcut() {
    auto optParams = getParams();
    if (!optParams)
        return;

    ScoreboardPainterParams params = *optParams;

    osaekomiClick(params, MatchStore::PlayerIndex::BLUE);
}

void ScoreOperatorWidget::cancelIppon(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canCancelIppon(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<CancelIpponAction>(params.match.getCombinedId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::cancelWazari(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canCancelWazari(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<CancelWazariAction>(params.match.getCombinedId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::cancelShido(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canCancelShido(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<CancelShidoAction>(params.match.getCombinedId(), playerIndex, params.masterTime));
}

void ScoreOperatorWidget::cancelHansokuMake(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canCancelHansokuMake(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<CancelHansokuMakeAction>(params.match.getCombinedId(), playerIndex, params.masterTime));
}

