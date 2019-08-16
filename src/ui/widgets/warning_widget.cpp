#include <QPainter>

#include "ui/widgets/colors.hpp"
#include "ui/widgets/warning_widget.hpp"

WarningWidget::WarningWidget(const QString &warning, QWidget *parent)
    : QWidget(parent)
    , mWarning(warning)
{
    setMinimumHeight(150);
    setMaximumHeight(150);
}

void WarningWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_UI_WARNING_BACKGROUND);
    painter.drawRect(rect());

    // Draw left border
    QRect borderRect(0, 0, 6, height());
    painter.setBrush(COLOR_UI_WARNING_FOREGROUND);
    painter.drawRect(borderRect);

    // Draw warning icon
    const int circleRadius = 35;
    QRect circleRect(40, height() / 2 - circleRadius, circleRadius*2, circleRadius*2);
    painter.drawEllipse(circleRect);

    painter.setBrush(COLOR_UI_WARNING_BACKGROUND);
    const int exclamationWidth = 6;
    int exclamationHeight = circleRadius * 0.9;
    int exclamationOffset = height() / 2 - exclamationHeight/2;
    QRect exclamationLine(40 + circleRadius - exclamationWidth/2, exclamationOffset, exclamationWidth, exclamationHeight - exclamationWidth - 5);
    QRect exclamationDot(40 + circleRadius - exclamationWidth/2, exclamationOffset + exclamationHeight - exclamationWidth, exclamationWidth, exclamationWidth);
    painter.drawRect(exclamationLine);
    painter.drawRect(exclamationDot);

    // Draw warning message
    QFont font("Noto Sans");
    font.setBold(true);
    font.setPointSize(20);
    painter.setFont(font);

    painter.setPen(COLOR_UI_WARNING_FOREGROUND);

    int textXOffset = 40 * 2 + circleRadius * 2;
    QRect textRect(textXOffset, 0, width() - textXOffset - 6, height());
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, mWarning);
}
