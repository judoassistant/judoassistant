#include "core/exception.hpp"
#include "ui/widgets/score_display_widget.hpp"
#include "ui/widgets/score_display_window.hpp"

ScoreDisplayWindow::ScoreDisplayWindow(const StoreManager &storeManager)
    : mStoreManager(storeManager)
{
    mScoreWidget = new ScoreDisplayWidget(mStoreManager, ScoreDisplayMode::DISPLAY, this);
    setCentralWidget(mScoreWidget);

    setWindowTitle(tr("JudoAssistant Score (Display)"));
}

ScoreDisplayWidget& ScoreDisplayWindow::getDisplayWidget() {
    return *mScoreWidget;
}

