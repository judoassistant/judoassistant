#pragma once

#include <QFont>

#include "core/stores/match_store.hpp"
#include "core/stores/player_store.hpp"
#include "ui/widgets/scoreboard_painters/scoreboard_painter.hpp"

class NationalScoreboardPainter : public ScoreboardPainter {
public:
    NationalScoreboardPainter();

    void paintEmpty(QPainter &painter, const QRect &rect) override;
    void paintIntroduction(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) override;
    void paintNormal(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) override;
    void paintWinner(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) override;

private:
    static constexpr int PADDING = 5;

    void paintIntroductionPlayer(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex);
    void paintIntroductionLower(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params);

    void paintNormalPlayer(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params, MatchStore::PlayerIndex playerIndex);
    void paintNormalLower(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params);

    QFont mFont;
};

