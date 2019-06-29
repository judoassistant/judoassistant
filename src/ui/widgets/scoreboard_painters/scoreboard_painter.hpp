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
    virtual void paintControls(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params);
    virtual void paintWinner(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) = 0;

    virtual void resizeEvent(const QRect &rect) = 0;

protected:
    QRect mDurationRect;
    QRect mWhiteOsaekomiRect;
    QRect mBlueOsaekomiRect;

    QRect mWhiteIpponRect;
    QRect mWhiteWazariRect;

    QRect mBlueIpponRect;
    QRect mBlueWazariRect;

    QRect mWhiteShidoRect;
    QRect mWhiteHansokuRect;

    QRect mBlueShidoRect;
    QRect mBlueHansokuRect;

    QRect mWhiteSecondShidoRect;
    QRect mBlueSecondShidoRect;
};

