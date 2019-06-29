#include <QPainter>

#include "ui/widgets/colors.hpp"
#include "ui/widgets/scoreboard_painters/scoreboard_painter.hpp"

void ScoreboardPainter::paintControls(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params) {
    painter.setPen(COLOR_SCOREBOARD_CONTROLS);
    painter.setBrush(Qt::NoBrush);

    painter.drawRect(mDurationRect);
    painter.drawRect(mWhiteIpponRect);
    painter.drawRect(mWhiteWazariRect);
    painter.drawRect(mWhiteOsaekomiRect);
    painter.drawRect(mWhiteShidoRect);
    painter.drawRect(mWhiteHansokuRect);

    painter.drawRect(mBlueIpponRect);
    painter.drawRect(mBlueWazariRect);
    painter.drawRect(mBlueOsaekomiRect);
    painter.drawRect(mBlueShidoRect);
    painter.drawRect(mBlueHansokuRect);
}
