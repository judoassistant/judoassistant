#pragma once

#include <QWidget>
#include <QFont>

class QPainter;

class ScoreDisplayWidget : public QWidget {
    Q_OBJECT
public:
    ScoreDisplayWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
private:
    void paintFirstCompetitor(QPainter &painter);
    void paintSecondCompetitor(QPainter &painter);
    void paintLowerSection(QPainter &painter);

    QFont mFont;
};

