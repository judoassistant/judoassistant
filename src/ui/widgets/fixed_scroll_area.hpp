#pragma once

#include <vector>

#include <QHBoxLayout>
#include <QScrollArea>

class FixedScrollArea : public QScrollArea {
    Q_OBJECT
public:
    FixedScrollArea(int cols, QWidget *parent = nullptr);
    void addWidget(QWidget *widget);
    void clear();
    void adaptSize();

private:
    int mColWidth;
    QWidget *mContentWidget;
    QHBoxLayout *mLayout;
    std::vector<QWidget*> mWidgets;
};
