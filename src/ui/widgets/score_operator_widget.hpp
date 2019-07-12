#pragma once

#include "ui/widgets/score_display_widget.hpp"

class ScoreOperatorWidget : public ScoreDisplayWidget {
    Q_OBJECT
public:
    ScoreOperatorWidget(StoreManager &storeManager, QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event) override;
    void paintControls(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params);
    void mouseReleaseEvent(QMouseEvent *event) override;

    void pausingTimerHit();
private:
    void durationShortcut();
    std::optional<ScoreboardPainterParams> getParams();

    void durationClick(ScoreboardPainterParams &params);
    void osaekomiClick(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex);
    void awardIppon(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex);
    void awardWazari(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex);
    void awardShido(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex);
    void awardHansokuMake(ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex);

    static constexpr auto PAUSING_TIMER_INTERVAL = std::chrono::milliseconds(100);

    StoreManager &mStoreManager;
    QTimer mPausingTimer;
};

