#include <QPainter>
#include <QMouseEvent>

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

    ScoreboardPainterParams params{category, match, whitePlayer, bluePlayer, mStoreManager.masterTime()};

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
    mStoreManager.dispatch(std::make_unique<AwardIpponAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));

    const auto &ruleset = params.category.getRuleset();
    if (ruleset.shouldPause(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));
}

void ScoreOperatorWidget::awardWazari(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    mStoreManager.dispatch(std::make_unique<AwardWazariAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));

    const auto &ruleset = params.category.getRuleset();
    if (ruleset.shouldPause(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));

}

void ScoreOperatorWidget::awardShido(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {
    mStoreManager.dispatch(std::make_unique<AwardShidoAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));

    const auto &ruleset = params.category.getRuleset();
    if (ruleset.shouldPause(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));

}

void ScoreOperatorWidget::awardHansokuMake(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex) {

    mStoreManager.dispatch(std::make_unique<AwardHansokuMakeAction>(params.category.getId(), params.match.getId(), playerIndex, params.masterTime));

    const auto &ruleset = params.category.getRuleset();
    if (ruleset.shouldPause(params.match, params.masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(params.category.getId(), params.match.getId(), params.masterTime));

}

