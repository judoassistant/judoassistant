#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QShortcut>

#include "core/actions/award_hansoku_make_action.hpp"
#include "core/actions/award_ippon_action.hpp"
#include "core/actions/award_shido_action.hpp"
#include "core/actions/award_wazari_action.hpp"
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
    if (ruleset.canAwardIppon(params.match, MatchStore::PlayerIndex::WHITE))
        painter.drawRect(mScoreboardPainter->getWhiteIpponRect());
    if (ruleset.canAwardWazari(params.match, MatchStore::PlayerIndex::WHITE))
        painter.drawRect(mScoreboardPainter->getWhiteWazariRect());
    if (ruleset.canStartOsaekomi(params.match, MatchStore::PlayerIndex::WHITE))
        painter.drawRect(mScoreboardPainter->getWhiteOsaekomiRect());
    if (ruleset.canAwardShido(params.match, MatchStore::PlayerIndex::WHITE))
        painter.drawRect(mScoreboardPainter->getWhiteShidoRect());
    if (ruleset.canAwardHansokuMake(params.match, MatchStore::PlayerIndex::WHITE))
        painter.drawRect(mScoreboardPainter->getWhiteHansokuRect());

    // Blue Player Controls
    if (ruleset.canAwardIppon(params.match, MatchStore::PlayerIndex::BLUE))
        painter.drawRect(mScoreboardPainter->getBlueIpponRect());
    if (ruleset.canAwardWazari(params.match, MatchStore::PlayerIndex::BLUE))
        painter.drawRect(mScoreboardPainter->getBlueWazariRect());
    if (ruleset.canStartOsaekomi(params.match, MatchStore::PlayerIndex::BLUE))
        painter.drawRect(mScoreboardPainter->getBlueOsaekomiRect());
    if (ruleset.canAwardShido(params.match, MatchStore::PlayerIndex::BLUE))
        painter.drawRect(mScoreboardPainter->getBlueShidoRect());
    if (ruleset.canAwardHansokuMake(params.match, MatchStore::PlayerIndex::BLUE))
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

    if (mScoreboardPainter->getDurationRect().contains(pos))
        durationClick(params);
    else if (mScoreboardPainter->getWhiteOsaekomiRect().contains(pos))
        osaekomiClick(params, MatchStore::PlayerIndex::WHITE);
    else if (mScoreboardPainter->getWhiteIpponRect().contains(pos))
        awardIppon(params, MatchStore::PlayerIndex::WHITE);
    else if (mScoreboardPainter->getWhiteWazariRect().contains(pos))
        awardWazari(params, MatchStore::PlayerIndex::WHITE);
    else if (mScoreboardPainter->getWhiteShidoRect().contains(pos))
        awardShido(params, MatchStore::PlayerIndex::WHITE);
    else if (mScoreboardPainter->getWhiteHansokuRect().contains(pos))
        awardHansokuMake(params, MatchStore::PlayerIndex::WHITE);
    else if (mScoreboardPainter->getBlueOsaekomiRect().contains(pos))
        osaekomiClick(params, MatchStore::PlayerIndex::BLUE);
    else if (mScoreboardPainter->getBlueIpponRect().contains(pos))
        awardIppon(params, MatchStore::PlayerIndex::BLUE);
    else if (mScoreboardPainter->getBlueWazariRect().contains(pos))
        awardWazari(params, MatchStore::PlayerIndex::BLUE);
    else if (mScoreboardPainter->getBlueShidoRect().contains(pos))
        awardShido(params, MatchStore::PlayerIndex::BLUE);
    else if (mScoreboardPainter->getBlueHansokuRect().contains(pos))
        awardHansokuMake(params, MatchStore::PlayerIndex::BLUE);
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
        auto masterTime = mStoreManager.masterTime();
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
        mStoreManager.dispatch(std::make_unique<StartOsaekomiAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));

        if (params.match.getStatus() != MatchStatus::UNPAUSED && (params.match.getDuration() < ruleset.getNormalTime() || params.match.isGoldenScore()))
            mStoreManager.dispatch(std::make_unique<ResumeMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));

    }
    else {
        // Stop osaekomi
        mStoreManager.dispatch(std::make_unique<StopOsaekomiAction>(params.category.getId(), params.match.getId(), params.masterTime));

        if (ruleset.shouldPause(params.match, params.masterTime))
            mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));
    }
}

void ScoreOperatorWidget::awardIppon(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardIppon(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<AwardIpponAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));

    if (ruleset.shouldPause(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));
}

void ScoreOperatorWidget::awardWazari(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardWazari(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<AwardWazariAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));

    if (ruleset.shouldPause(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));
}

void ScoreOperatorWidget::awardShido(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardShido(params.match, playerIndex))
        return;

    mStoreManager.dispatch(std::make_unique<AwardShidoAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));

    if (ruleset.shouldPause(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));
}

void ScoreOperatorWidget::awardHansokuMake(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    const Ruleset &ruleset = params.category.getRuleset();
    if (!ruleset.canAwardHansokuMake(params.match, playerIndex))
        return;

    auto reply = QMessageBox::question(this, tr("Would you like to award direct hansoku-make?"), tr("Are you sure you would like to award direct hansoku-make?"), QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel)
        return;

    mStoreManager.dispatch(std::make_unique<AwardHansokuMakeAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));

    if (ruleset.shouldPause(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));
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

