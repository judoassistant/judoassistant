#pragma once

#include "ui/widgets/score_display_widget.hpp"

class ScoreOperatorWidget : public ScoreDisplayWidget {
    Q_OBJECT
public:
    ScoreOperatorWidget(const StoreManager &storeManager, QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event) override;
    void paintControls(QPainter &painter, const QRect &rect, const ScoreboardPainterParams &params);
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
};

