#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QGroupBox>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>

#include "core/actions/reset_matches_action.hpp"
#include "core/log.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "ui/constants/homepage.hpp"
#include "ui/misc/dark_palette.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/score_operator_window.hpp"

ScoreOperatorWindow::ScoreOperatorWindow()
    : mDisplayWindow(mStoreManager)
{
    QWidget *centralWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    createStatusBar();
    createTournamentMenu();
    createEditMenu();
    createViewMenu();
    createPreferencesMenu();
    createHelpMenu();

    layout->addWidget(createScoreboardSection(), 1); // stretch=1 such that all remaining space is filled by this widget
    layout->addWidget(createLowerSection());
    setCentralWidget(centralWidget);

    setWindowTitle(tr("JudoAssistant Score"));

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &ScoreOperatorWindow::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &ScoreOperatorWindow::endResetTournament);

    findNextMatch();
    disableControlButtons();
}

void ScoreOperatorWindow::createStatusBar() {
    statusBar();
}

void ScoreOperatorWindow::createTournamentMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Tournament"));

    mConnectAction = new QAction(tr("Connect.."), this);
    mConnectAction->setShortcuts(QKeySequence::New);
    mConnectAction->setStatusTip(tr("Connect to hub"));
    connect(mConnectAction, &QAction::triggered, this, &ScoreOperatorWindow::showConnectDialog);
    menu->addAction(mConnectAction);

    mDisconnectAction = new QAction(tr("Disconnect"), this);
    mDisconnectAction->setShortcuts(QKeySequence::New);
    mDisconnectAction->setStatusTip(tr("Disconnect from hub"));
    connect(mDisconnectAction, &QAction::triggered, this, &ScoreOperatorWindow::disconnect);
    menu->addAction(mDisconnectAction);

    changeNetworkClientState(mStoreManager.getNetworkClientState());
    connect(&mStoreManager.getNetworkClient(), &NetworkClient::stateChanged, this, &ScoreOperatorWindow::changeNetworkClientState);
}

void ScoreOperatorWindow::createEditMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Edit"));

    mTatamiMenu = menu->addMenu(tr("Tatami"));

    {
        // QAction *englishAction = new QAction(tr("English"), this);
        // submenu->addAction(englishAction);
        // populateTatamiMenu();
    }
    // {
    //     QAction *action = new QAction(tr("Undo"), this);
    //     action->setShortcuts(QKeySequence::Undo);
    //     action->setStatusTip(tr("Undo last action"));
    //     action->setEnabled(mStoreManager.canUndo());
    //     menu->addAction(action);
    //     connect(&mStoreManager, &MasterStoreManager::undoStatusChanged, action, &QAction::setEnabled);
    //     connect(action, &QAction::triggered, &mStoreManager, &MasterStoreManager::undo);
    // }

    // {
    //     QAction *action = new QAction(tr("Redo"), this);
    //     action->setShortcuts(QKeySequence::Redo);
    //     action->setStatusTip(tr("Redo the last undone action"));
    //     action->setEnabled(mStoreManager.canRedo());
    //     menu->addAction(action);
    //     connect(&mStoreManager, &MasterStoreManager::redoStatusChanged, action, &QAction::setEnabled);
    //     connect(action, &QAction::triggered, &mStoreManager, &MasterStoreManager::redo);
    // }
}

void ScoreOperatorWindow::clearTatamiMenu() {
    // delete mTatamiActionGroup;
    mTatamiActionGroup = nullptr;
    mTatamiMenu->clear();
}

void ScoreOperatorWindow::populateTatamiMenu() {
    const auto &tatamis = mStoreManager.getTournament().getTatamis();

    mTatamiActionGroup = new QActionGroup(this);
    for (size_t i = 0; i < tatamis.tatamiCount(); ++i) {
        TatamiLocation location = {tatamis.getHandle(i)};

        QAction *action = new QAction(tr("Tatami %1").arg(QString::number(i+1)), mTatamiMenu);
        action->setCheckable(true);
        mTatamiActionGroup->addAction(action);
        if (mTatami && location == *mTatami) {
            action->setChecked(true);
        }

        connect(action, &QAction::triggered, this, [=]() { setTatami(location); });

        mTatamiMenu->addAction(action);
    }
}

void ScoreOperatorWindow::createViewMenu() {
    // QMenu *menu = menuBar()->addMenu(tr("View"));
}

void ScoreOperatorWindow::createPreferencesMenu() {
    // TODO: Consider refactoring this into super class
    QMenu *menu = menuBar()->addMenu(tr("Preferences"));
    {
        QMenu *submenu = menu->addMenu("Language");
        QAction *englishAction = new QAction(tr("English"), this);
        submenu->addAction(englishAction);
    }
    {
        {
            DarkPalette palette;
            QApplication::setPalette(palette);
            setPalette(palette);
        }

        QMenu *submenu = menu->addMenu("Color Scheme");
        auto *actionGroup = new QActionGroup(this);

        QAction *darkAction = new QAction(tr("Dark"), this);
        darkAction->setCheckable(true);
        darkAction->setChecked(true);
        actionGroup->addAction(darkAction);

        connect(darkAction, &QAction::triggered, [this]() {
            DarkPalette palette;
            setPalette(palette);
        });

        QAction *lightAction = new QAction(tr("Light"), this);
        lightAction->setCheckable(true);
        actionGroup->addAction(lightAction);

        connect(lightAction, &QAction::triggered, [this]() {
            auto palette = this->style()->standardPalette();
            QApplication::setPalette(palette);
            setPalette(palette);
        });

        submenu->addAction(darkAction);
        submenu->addAction(lightAction);
    }
}

void ScoreOperatorWindow::createHelpMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Help"));
    {
        QAction *action = new QAction(tr("JudoAssistant Home Page"), this);
        connect(action, &QAction::triggered, this, &ScoreOperatorWindow::openHomePage);
        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("JudoAssistant Manual"), this);
        connect(action, &QAction::triggered, this, &ScoreOperatorWindow::openManual);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Report an Issue"), this);
        connect(action, &QAction::triggered, this, &ScoreOperatorWindow::openReportIssue);

        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("About"), this);
        connect(action, &QAction::triggered, this, &ScoreOperatorWindow::showAboutDialog);
        menu->addAction(action);
    }
}

void ScoreOperatorWindow::openHomePage() {
    if(QDesktopServices::openUrl(Constants::HOME_PAGE_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open home page."));
}

void ScoreOperatorWindow::openManual() {
    if(QDesktopServices::openUrl(Constants::MANUAL_URL))
        return;

    QMessageBox::warning(this, tr("Unable to open manual."), tr("The manual is available at"));
}

void ScoreOperatorWindow::openReportIssue() {
    if(QDesktopServices::openUrl(Constants::REPORT_ISSUE_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open report issue page."));
}

void ScoreOperatorWindow::quit() {
    QCoreApplication::exit();
}

void ScoreOperatorWindow::showAboutDialog() {
    QMessageBox::about(this, tr("JudoAssistant - About"), tr("TODO"));
}

QWidget* ScoreOperatorWindow::createLowerSection() {
    QWidget *res = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout;

    {
        QGroupBox *matchBox = new QGroupBox("Match Controls", res);
        QVBoxLayout *subLayout = new QVBoxLayout(res);

        mNextButton = new QPushButton("Go to next match", matchBox);
        connect(mNextButton, &QPushButton::clicked, this, &ScoreOperatorWindow::goNextMatch);

        mResetButton = new QPushButton("Reset Match", matchBox);
        connect(mResetButton, &QPushButton::clicked, this, &ScoreOperatorWindow::resetButtonClick);

        subLayout->addWidget(mNextButton);
        subLayout->addWidget(mResetButton);

        matchBox->setLayout(subLayout);
        layout->addWidget(matchBox);
    }

    {
        QGroupBox *nextMatchBox = new QGroupBox("Next Match", res);
        QVBoxLayout *nextMatchLayout = new QVBoxLayout(nextMatchBox);
        mNextMatchWidget = new MatchCardWidget(mStoreManager, nextMatchBox);
        nextMatchLayout->addWidget(mNextMatchWidget);

        layout->addWidget(nextMatchBox);
    }

    res->setLayout(layout);
    return res;
}

QWidget* ScoreOperatorWindow::createScoreboardSection() {
    QGroupBox *viewBox = new QGroupBox("Spectator View");
    QVBoxLayout *subLayout = new QVBoxLayout;

    mScoreOperatorWidget = new ScoreOperatorWidget(mStoreManager, viewBox);
    subLayout->addWidget(mScoreOperatorWidget);

    viewBox->setLayout(subLayout);

    return viewBox;
}

void ScoreOperatorWindow::beginResetTournament() {
    while (!mConnections.empty()) {
        QMainWindow::disconnect(mConnections.top());
        mConnections.pop();
    }
}

void ScoreOperatorWindow::endResetTournament() {
    auto &tournament = mStoreManager.getTournament();
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeAdded, this, &ScoreOperatorWindow::clearTatamiMenu));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAdded, this, &ScoreOperatorWindow::populateTatamiMenu));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisAboutToBeErased, this, &ScoreOperatorWindow::clearTatamiMenu));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisErased, this, &ScoreOperatorWindow::populateTatamiMenu));
    mConnections.push(connect(&tournament, &QTournamentStore::tatamisChanged, this, &ScoreOperatorWindow::changeTatamis));

    mConnections.push(connect(&tournament, &QTournamentStore::matchesChanged, this, &ScoreOperatorWindow::changeMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesAboutToBeReset, this, &ScoreOperatorWindow::beginResetMatches));
    mConnections.push(connect(&tournament, &QTournamentStore::matchesReset, this, &ScoreOperatorWindow::endResetMatches));

    clearTatamiMenu();
    populateTatamiMenu();
    findNextMatch();
    updateControlButtons();
}

void ScoreOperatorWindow::setTatami(TatamiLocation tatami) {
    mTatami = tatami;

    findNextMatch();
}

void ScoreOperatorWindow::silentConnect(QString host, int port) {
    mStoreManager.connect(host, port);
}

void ScoreOperatorWindow::changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {
    if (!mTatami)
        return;

    for (const auto &location: locations) {
        if (location.getTatamiHandle() == mTatami->handle) {
            findNextMatch();
            return;
        }
    }
}

void ScoreOperatorWindow::findNextMatch() {
    const auto &tournament = mStoreManager.getTournament();
    const auto &tatamis = tournament.getTatamis();

    if (mTatami && tatamis.containsTatami(*mTatami)) {
        const auto &tatami = tatamis.at(*mTatami);

        for (size_t i = 0; i < tatami.groupCount(); ++i) {
            const auto &group = tatami.at(i);
            if (group.getStatus() == ConcurrentBlockGroup::Status::FINISHED) continue;

            for (auto combinedId : group.getMatches()) {
                if (mCurrentMatch == combinedId) {
                    continue;
                }

                const auto &match = tournament.getCategory(combinedId.first).getMatch(combinedId.second);

                if (match.isBye())
                    continue;

                if (match.getStatus() != MatchStatus::FINISHED) {
                    mNextMatch = combinedId;
                    mNextMatchWidget->setMatch(combinedId);
                    updateNextButton();
                    return;
                }
            }
        }
    }

    mNextMatch = std::nullopt;
    mNextMatchWidget->setMatch(std::nullopt);
    updateNextButton();
}

void ScoreOperatorWindow::changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {
    // handle next match changing
    bool shouldUpdateNextButton = false;
    if (mNextMatch.has_value() && mNextMatch->first == categoryId) {
        if(std::find(matchIds.begin(), matchIds.end(), mNextMatch->second) != matchIds.end()) {
            shouldUpdateNextButton = true;
        }
    }

    if (mCurrentMatch.has_value() && mCurrentMatch->first == categoryId) {
        shouldUpdateNextButton = true;
        updateControlButtons();
    }

    if (shouldUpdateNextButton)
        updateNextButton();
}

void ScoreOperatorWindow::beginResetMatches(const std::vector<CategoryId> &categoryIds) {
    // next match category resetting is handled by tatami hooks
    if (!mCurrentMatch)
        return;

    for (auto categoryId : categoryIds) {
        if (mCurrentMatch->first == categoryId) {
            disableControlButtons(); // disable controls
            return;
        }
    }
}

void ScoreOperatorWindow::endResetMatches(const std::vector<CategoryId> &categoryIds) {
    // next match category resetting is handled by tatami hooks
    updateControlButtons();
}

void ScoreOperatorWindow::updateNextButton() {
    const auto &tournament = mStoreManager.getTournament();
    bool enabled = true;
    if (!mNextMatch.has_value()) {
        enabled = false;
    }
    else {
        const auto &match = tournament.getCategory(mNextMatch->first).getMatch(mNextMatch->second);
        if (!match.getWhitePlayer() || !match.getBluePlayer())
            enabled = false;
    }

    mNextButton->setEnabled(enabled);
}

void ScoreOperatorWindow::goNextMatch() {
    // Check if next match is valid
    if (!mNextMatch.has_value())
        return;

    const auto &tournament = mStoreManager.getTournament();
    const auto &nextMatch = tournament.getCategory(mNextMatch->first).getMatch(mNextMatch->second);
    if (!nextMatch.getWhitePlayer() || !nextMatch.getBluePlayer())
        return;

    // Check if current match is finished
    if (mCurrentMatch.has_value()) {
        if (tournament.containsCategory(mCurrentMatch->first)) {
            const auto &category = tournament.getCategory(mCurrentMatch->first);
            if (category.containsMatch(mCurrentMatch->second)) {
                const auto &match = category.getMatch(mCurrentMatch->second);
                if (match.getStatus() != MatchStatus::FINISHED) {
                    auto reply = QMessageBox::question(this, tr("Go to next match?"), tr("Are you sure you would like to go to the next match? The current match is not finished."), QMessageBox::Yes | QMessageBox::Cancel);
                    if (reply == QMessageBox::Cancel)
                        return;
                }
            }
        }
    }

    mCurrentMatch = mNextMatch;
    mScoreOperatorWidget->setMatch(mCurrentMatch);
    mDisplayWindow.getDisplayWidget().setMatch(mCurrentMatch);
    findNextMatch();
    updateControlButtons();
}

void ScoreOperatorWindow::disableControlButtons() {
    mResetButton->setEnabled(false);
}

void ScoreOperatorWindow::updateControlButtons() {
    if (!mCurrentMatch)
        disableControlButtons();
    else
        mResetButton->setEnabled(true);
}


void ScoreOperatorWindow::changeNetworkClientState(NetworkClientState state) {
    mConnectAction->setEnabled(state == NetworkClientState::NOT_CONNECTED);
    mDisconnectAction->setEnabled(state == NetworkClientState::CONNECTED);
}

void ScoreOperatorWindow::show() {
    ClientWindow::show();
    // mDisplayWindow.show();
}

void ScoreOperatorWindow::resetButtonClick() {
    auto reply = QMessageBox::question(this, tr("Would you like to reset the match?"), tr("Are you sure you would like to reset the match?"), QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel)
        return;

    if (!mCurrentMatch)
        return;

    mStoreManager.dispatch(std::make_unique<ResetMatchesAction>(mCurrentMatch->first, mCurrentMatch->second));
}

