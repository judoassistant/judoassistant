#include "ui/widgets/scoreboard_painters/scoreboard_painter.hpp"

const QRect& ScoreboardPainter::getDurationRect() {
    return mDurationRect;
}

void ScoreboardPainter::setDurationRect(const QRect &rect) {
    mDurationRect = rect;
}

