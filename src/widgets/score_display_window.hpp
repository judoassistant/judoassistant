#pragma once

#include <QMainWindow>

#include "core.hpp"
#include "widgets/score_display_widget.hpp"

class ScoreDisplayWindow : public QMainWindow {
    Q_OBJECT

public:
    ScoreDisplayWindow();

private slots:
private:
    ScoreDisplayWidget *mScoreWidget;
};

