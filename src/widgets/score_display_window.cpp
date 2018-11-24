#include "widgets/score_display_window.hpp"

#include "widgets/score_display_widget.hpp"
#include "exception.hpp"

ScoreDisplayWindow::ScoreDisplayWindow()
{
    mScoreWidget = new ScoreDisplayWidget(this);
    setCentralWidget(mScoreWidget);

    setWindowTitle(tr("JudoAssistant Score (Display)"));
}
