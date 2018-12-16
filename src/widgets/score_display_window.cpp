#include "widgets/score_display_window.hpp"

#include "widgets/score_display_widget.hpp"
#include "exception.hpp"

ScoreDisplayWindow::ScoreDisplayWindow(const StoreManager &storeManager)
    : mStoreManager(storeManager)
{
    mScoreWidget = new ScoreDisplayWidget(mStoreManager, this);
    setCentralWidget(mScoreWidget);

    setWindowTitle(tr("JudoAssistant Score (Display)"));
}
