#pragma once

#include <chrono>
#include <QRect>
#include "core/core.hpp"

class QPainter;
class CategoryStore;
class MatchStore;
class PlayerStore;
class QRect;

// Encapsulates all parameters used for drawing a scoreboard
struct ScoreboardPainterParams {
    const CategoryStore &category;
    const MatchStore &match;
    const PlayerStore &whitePlayer;
    const PlayerStore &bluePlayer;
    std::chrono::milliseconds masterTime;
};

class ScoreboardPainter {
public:
    virtual ~ScoreboardPainter() = default;

    virtual void paintEmpty(QPainter &painter, const QRect &rect) = 0;
    virtual void paintIntroduction(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) = 0;
    virtual void paintNormal(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) = 0;
    virtual void paintWinner(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) = 0;

    virtual void resizeEvent(const QRect &rect) = 0;

    const QRect& getDurationRect();

protected:
    QRect mDurationRect;
    QRect mOsaekomiRect;

    QRect mWhiteScoreRect;
    QRect mBlueScoreRect;

    QRect mWhiteFirstPenaltyRect;
    QRect mBlueFirstPenaltyRect;

    QRect mWhiteSecondPenaltyRect;
    QRect mBlueSecondPenaltyRect;
};

