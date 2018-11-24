#pragma once

#include <QMainWindow>

#include "core.hpp"

class ScoreDisplayWidget;

class ScoreDisplayWindow : public QMainWindow {
    Q_OBJECT

public:
    ScoreDisplayWindow();

private slots:
private:
    ScoreDisplayWidget *mScoreWidget;
};

