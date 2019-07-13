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
    bool blink;
};

class ScoreboardPainter {
public:
    virtual ~ScoreboardPainter() = default;

    virtual void paintEmpty(QPainter &painter, const QRect &rect) = 0;
    virtual void paintIntroduction(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) = 0;
    virtual void paintNormal(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) = 0;
    virtual void paintWinner(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) = 0;

    virtual void resizeEvent(const QRect &rect) = 0;

    const QRect& getDurationRect() const;
    const QRect& getWhiteOsaekomiRect() const;
    const QRect& getBlueOsaekomiRect() const;
    const QRect& getWhiteIpponRect() const;
    const QRect& getWhiteWazariRect() const;
    const QRect& getBlueIpponRect() const;
    const QRect& getBlueWazariRect() const;
    const QRect& getWhiteShidoRect() const;
    const QRect& getWhiteHansokuRect() const;
    const QRect& getBlueShidoRect() const;
    const QRect& getBlueHansokuRect() const;

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
};

