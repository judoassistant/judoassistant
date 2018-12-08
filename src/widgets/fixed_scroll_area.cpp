#include "widgets/fixed_scroll_area.hpp"

FixedScrollArea::FixedScrollArea(int colWidth, QWidget *parent)
    : QScrollArea(parent)
    , mColWidth(colWidth)
{
    setWidgetResizable(true);
    mContentWidget = new QWidget(this);
    setWidget(mContentWidget);
    mLayout = new QHBoxLayout(mContentWidget);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void FixedScrollArea::addWidget(QWidget *widget) {
    mWidgets.push_back(widget);
    mLayout->addWidget(widget);
    adaptSize();
}

void FixedScrollArea::clear() {
    for (QWidget *widget : mWidgets) {
        mLayout->removeWidget(widget);
        delete widget;
    }

    mWidgets.clear();
    adaptSize();
}

void FixedScrollArea::adaptSize() {
    int spacing = 5;
    // int w = 1.0*(width() - spacing*(mCols+1.6))/mCols;
    int wCorrected = mColWidth*mWidgets.size() + spacing*(mWidgets.size()+2);
    mContentWidget->setFixedWidth(wCorrected);

//     mContentWidget->setFixedHeight(viewport()->height());
}

