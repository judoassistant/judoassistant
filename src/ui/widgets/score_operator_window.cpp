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

#include "core/actions/award_hansoku_make_action.hpp"
#include "core/actions/award_ippon_action.hpp"
#include "core/actions/award_shido_action.hpp"
#include "core/actions/award_wazari_action.hpp"
#include "core/actions/pause_match_action.hpp"
#include "core/actions/reset_matches_action.hpp"
#include "core/actions/resume_match_action.hpp"
#include "core/actions/start_osaekomi_action.hpp"
#include "core/actions/stop_osaekomi_action.hpp"
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
    layout->addWidget(createButtonSection());
    layout->addWidget(createLowerSection());
    setCentralWidget(centralWidget);

    setWindowTitle(tr("JudoAssistant Score"));

    connect(&mStoreManager, &StoreManager::tournamentAboutToBeReset, this, &ScoreOperatorWindow::beginResetTournament);
    connect(&mStoreManager, &StoreManager::tournamentReset, this, &ScoreOperatorWindow::endResetTournament);

    findNextMatch();
    disableControlButtons();

    connect(&mPausingTimer, &QTimer::timeout, this, &ScoreOperatorWindow::pausingTimerHit);
    mPausingTimer.start(PAUSING_TIMER_INTERVAL);
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
        mResumeButton = new QPushButton("Resume Match", matchBox);
        connect(mResumeButton, &QPushButton::clicked, this, &ScoreOperatorWindow::resumeButtonClick);
        mResetButton = new QPushButton("Reset Match", matchBox);
        connect(mResetButton, &QPushButton::clicked, this, &ScoreOperatorWindow::resetButtonClick);

        subLayout->addWidget(mNextButton);
        subLayout->addWidget(mResumeButton);
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

    mScoreDisplayWidget = new ScoreDisplayWidget(mStoreManager, ScoreDisplayMode::OPERATOR, viewBox);
    subLayout->addWidget(mScoreDisplayWidget);

    viewBox->setLayout(subLayout);

    return viewBox;
}

QWidget* ScoreOperatorWindow::createButtonSection() {
    QWidget *res = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout;

    {
        QGroupBox *whiteBox = new QGroupBox("White Player Controls", res);
        QVBoxLayout *subLayout = new QVBoxLayout(res);

        mWhiteIpponButton = new QPushButton("Award Ippon to White");
        connect(mWhiteIpponButton, &QPushButton::clicked, [this](){awardIppon(MatchStore::PlayerIndex::WHITE);});

        mWhiteWazariButton = new QPushButton("Award Wazari to White");
        connect(mWhiteWazariButton, &QPushButton::clicked, [this](){awardWazari(MatchStore::PlayerIndex::WHITE);});

        mWhiteShidoButton = new QPushButton("Award Shido to White");
        connect(mWhiteShidoButton, &QPushButton::clicked, [this](){awardShido(MatchStore::PlayerIndex::WHITE);});

        mWhiteHansokuMakeButton = new QPushButton("Award Hansoku Make to White");
        connect(mWhiteHansokuMakeButton, &QPushButton::clicked, [this](){awardHansokuMake(MatchStore::PlayerIndex::WHITE);});

        mWhiteOsaekomiButton = new QPushButton("Start Osaekomi for White");
        connect(mWhiteOsaekomiButton, &QPushButton::clicked, [this](){osaekomiButtonClick(MatchStore::PlayerIndex::WHITE);});

        subLayout->addWidget(mWhiteIpponButton);
        subLayout->addWidget(mWhiteWazariButton);
        subLayout->addWidget(mWhiteShidoButton);
        subLayout->addWidget(mWhiteHansokuMakeButton);
        subLayout->addWidget(mWhiteOsaekomiButton);

        whiteBox->setLayout(subLayout);
        layout->addWidget(whiteBox);
    }

    {
        QGroupBox *blueBox = new QGroupBox("Blue Player Controls", res);
        QVBoxLayout *subLayout = new QVBoxLayout(res);

        mBlueIpponButton = new QPushButton("Award Ippon to Blue");
        connect(mBlueIpponButton, &QPushButton::clicked, [this](){awardIppon(MatchStore::PlayerIndex::BLUE);});

        mBlueWazariButton = new QPushButton("Award Wazari to Blue");
        connect(mBlueWazariButton, &QPushButton::clicked, [this](){awardWazari(MatchStore::PlayerIndex::BLUE);});

        mBlueShidoButton = new QPushButton("Award Shido to Blue");
        connect(mBlueShidoButton, &QPushButton::clicked, [this](){awardShido(MatchStore::PlayerIndex::BLUE);});

        mBlueHansokuMakeButton = new QPushButton("Award Hansoku Make to Blue");
        connect(mBlueHansokuMakeButton, &QPushButton::clicked, [this](){awardHansokuMake(MatchStore::PlayerIndex::BLUE);});

        mBlueOsaekomiButton = new QPushButton("Start Osaekomi for Blue");
        connect(mBlueOsaekomiButton, &QPushButton::clicked, [this](){osaekomiButtonClick(MatchStore::PlayerIndex::BLUE);});

        subLayout->addWidget(mBlueIpponButton);
        subLayout->addWidget(mBlueWazariButton);
        subLayout->addWidget(mBlueShidoButton);
        subLayout->addWidget(mBlueHansokuMakeButton);
        subLayout->addWidget(mBlueOsaekomiButton);

        blueBox->setLayout(subLayout);
        layout->addWidget(blueBox);
    }

    res->setLayout(layout);
    return res;
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

    if (enabled && mCurrentMatch.has_value()) {
        if (tournament.containsCategory(mCurrentMatch->first)) {
            const auto &category = tournament.getCategory(mCurrentMatch->first);
            if (category.containsMatch(mCurrentMatch->second)) {
                const auto &match = category.getMatch(mCurrentMatch->second);
                if (match.getStatus() != MatchStatus::FINISHED)
                    enabled = false;
            }
        }
    }

    mNextButton->setEnabled(enabled);
}

void ScoreOperatorWindow::goNextMatch() {
    if (!mNextButton->isEnabled()) return;

    mCurrentMatch = mNextMatch;
    mScoreDisplayWidget->setMatch(mCurrentMatch);
    mDisplayWindow.getDisplayWidget().setMatch(mCurrentMatch);
    findNextMatch();
    updateControlButtons();
}

void ScoreOperatorWindow::disableControlButtons() {
    mResumeButton->setEnabled(false);
    mResumeButton->setText("Resume Match");

    mResetButton->setEnabled(false);

    mWhiteIpponButton->setEnabled(false);
    mWhiteWazariButton->setEnabled(false);
    mWhiteShidoButton->setEnabled(false);
    mWhiteHansokuMakeButton->setEnabled(false);

    mBlueIpponButton->setEnabled(false);
    mBlueWazariButton->setEnabled(false);
    mBlueShidoButton->setEnabled(false);
    mBlueHansokuMakeButton->setEnabled(false);

    mWhiteOsaekomiButton->setEnabled(false);
    mWhiteOsaekomiButton->setText("Start Osaekomi for White");

    mBlueOsaekomiButton->setEnabled(false);
    mBlueOsaekomiButton->setText("Start Osaekomi for Blue");
}

void ScoreOperatorWindow::updateControlButtons() {
    if (!mCurrentMatch) {
        disableControlButtons();
        return;
    }

    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCurrentMatch->first)) {
        disableControlButtons();
        return;
    }
    const auto &category = tournament.getCategory(mCurrentMatch->first);
    if (!category.containsMatch(mCurrentMatch->second)) {
        disableControlButtons();
        return;
    }
    const auto &match = category.getMatch(mCurrentMatch->second);
    if (!match.getWhitePlayer().has_value() || !match.getBluePlayer().has_value()) {
        disableControlButtons();
        return;
    }

    const auto &ruleset = category.getRuleset();
    auto masterTime = mStoreManager.masterTime();

    if (match.getStatus() == MatchStatus::UNPAUSED) {
        mResumeButton->setText("Pause match");
        mResumeButton->setEnabled(true);
    }
    else {
        mResumeButton->setText("Resume Match");
        mResumeButton->setEnabled(ruleset.canResume(match, masterTime));
    }

    mResetButton->setEnabled(true);

    mWhiteIpponButton->setEnabled(ruleset.canAddIppon(match, MatchStore::PlayerIndex::WHITE));
    mWhiteWazariButton->setEnabled(ruleset.canAddWazari(match, MatchStore::PlayerIndex::WHITE));
    mWhiteShidoButton->setEnabled(ruleset.canAddShido(match, MatchStore::PlayerIndex::WHITE));
    mWhiteHansokuMakeButton->setEnabled(ruleset.canAddHansokuMake(match, MatchStore::PlayerIndex::WHITE));

    mBlueIpponButton->setEnabled(ruleset.canAddIppon(match, MatchStore::PlayerIndex::BLUE));
    mBlueWazariButton->setEnabled(ruleset.canAddWazari(match, MatchStore::PlayerIndex::BLUE));
    mBlueShidoButton->setEnabled(ruleset.canAddShido(match, MatchStore::PlayerIndex::BLUE));
    mBlueHansokuMakeButton->setEnabled(ruleset.canAddHansokuMake(match, MatchStore::PlayerIndex::BLUE));

    auto osaekomi = match.getOsaekomi();
    if (osaekomi.has_value()) {
        if (osaekomi->first == MatchStore::PlayerIndex::WHITE) {
            mWhiteOsaekomiButton->setEnabled(ruleset.canStopOsaekomi(match, masterTime));
            mWhiteOsaekomiButton->setText("Stop Osaekomi for White");

            mBlueOsaekomiButton->setEnabled(ruleset.canStartOsaekomi(match, MatchStore::PlayerIndex::BLUE));
            mBlueOsaekomiButton->setText("Start Osaekomi for Blue");
        }
        else {
            mWhiteOsaekomiButton->setEnabled(ruleset.canStartOsaekomi(match, MatchStore::PlayerIndex::WHITE));
            mWhiteOsaekomiButton->setText("Start Osaekomi for White");

            mBlueOsaekomiButton->setEnabled(ruleset.canStopOsaekomi(match, masterTime));
            mBlueOsaekomiButton->setText("Stop Osaekomi for Blue");
        }
    }
    else {
        mWhiteOsaekomiButton->setEnabled(ruleset.canStartOsaekomi(match, MatchStore::PlayerIndex::WHITE));
        mWhiteOsaekomiButton->setText("Start Osaekomi for White");

        mBlueOsaekomiButton->setEnabled(ruleset.canStartOsaekomi(match, MatchStore::PlayerIndex::BLUE));
        mBlueOsaekomiButton->setText("Start Osaekomi for Blue");
    }
}

void ScoreOperatorWindow::resumeButtonClick() {
    if (!mCurrentMatch)
        return;

    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCurrentMatch->first))
        return;
    const auto &category = tournament.getCategory(mCurrentMatch->first);
    if (!category.containsMatch(mCurrentMatch->second))
        return;
    const auto &match = category.getMatch(mCurrentMatch->second);
    if (!match.getWhitePlayer().has_value() || !match.getBluePlayer().has_value())
        return;

    const auto &ruleset = category.getRuleset();

    if (match.getStatus() == MatchStatus::UNPAUSED) {
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(mCurrentMatch->first, mCurrentMatch->second, mStoreManager.masterTime()));
    }
    else {
        auto masterTime = mStoreManager.masterTime();
        if (!ruleset.canResume(match, masterTime))
            return;
        mStoreManager.dispatch(std::make_unique<ResumeMatchAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));
    }
}

void ScoreOperatorWindow::awardIppon(MatchStore::PlayerIndex playerIndex) {
    // TODO: Somehow avoid repeating these 14 lines in the functions
    if (!mCurrentMatch)
        return;

    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCurrentMatch->first))
        return;
    const auto &category = tournament.getCategory(mCurrentMatch->first);
    if (!category.containsMatch(mCurrentMatch->second))
        return;
    const auto &match = category.getMatch(mCurrentMatch->second);
    if (!match.getWhitePlayer().has_value() || !match.getBluePlayer().has_value())
        return;

    const auto &ruleset = category.getRuleset();

    auto masterTime = mStoreManager.masterTime();
    mStoreManager.dispatch(std::make_unique<AwardIpponAction>(mCurrentMatch->first, mCurrentMatch->second, playerIndex, masterTime));

    if (ruleset.shouldPause(match, masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));
}

void ScoreOperatorWindow::awardWazari(MatchStore::PlayerIndex playerIndex) {
    // TODO: Somehow avoid repeating these 14 lines in the functions
    if (!mCurrentMatch)
        return;

    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCurrentMatch->first))
        return;
    const auto &category = tournament.getCategory(mCurrentMatch->first);
    if (!category.containsMatch(mCurrentMatch->second))
        return;
    const auto &match = category.getMatch(mCurrentMatch->second);
    if (!match.getWhitePlayer().has_value() || !match.getBluePlayer().has_value())
        return;

    const auto &ruleset = category.getRuleset();

    auto masterTime = mStoreManager.masterTime();
    mStoreManager.dispatch(std::make_unique<AwardWazariAction>(mCurrentMatch->first, mCurrentMatch->second, playerIndex, masterTime));

    if (ruleset.shouldPause(match, masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));
}

void ScoreOperatorWindow::awardShido(MatchStore::PlayerIndex playerIndex) {
    if (!mCurrentMatch)
        return;

    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCurrentMatch->first))
        return;
    const auto &category = tournament.getCategory(mCurrentMatch->first);
    if (!category.containsMatch(mCurrentMatch->second))
        return;
    const auto &match = category.getMatch(mCurrentMatch->second);
    if (!match.getWhitePlayer().has_value() || !match.getBluePlayer().has_value())
        return;

    const auto &ruleset = category.getRuleset();

    auto masterTime = mStoreManager.masterTime();
    mStoreManager.dispatch(std::make_unique<AwardShidoAction>(mCurrentMatch->first, mCurrentMatch->second, playerIndex, masterTime));

    if (ruleset.shouldPause(match, masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));
}

void ScoreOperatorWindow::awardHansokuMake(MatchStore::PlayerIndex playerIndex) {
    if (!mCurrentMatch)
        return;

    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCurrentMatch->first))
        return;
    const auto &category = tournament.getCategory(mCurrentMatch->first);
    if (!category.containsMatch(mCurrentMatch->second))
        return;
    const auto &match = category.getMatch(mCurrentMatch->second);
    if (!match.getWhitePlayer().has_value() || !match.getBluePlayer().has_value())
        return;

    const auto &ruleset = category.getRuleset();

    auto masterTime = mStoreManager.masterTime();
    mStoreManager.dispatch(std::make_unique<AwardHansokuMakeAction>(mCurrentMatch->first, mCurrentMatch->second, playerIndex, masterTime));

    if (ruleset.shouldPause(match, masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));
}

void ScoreOperatorWindow::pausingTimerHit() {
    if (!mCurrentMatch)
        return;

    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCurrentMatch->first))
        return;
    const auto &category = tournament.getCategory(mCurrentMatch->first);
    if (!category.containsMatch(mCurrentMatch->second))
        return;
    const auto &match = category.getMatch(mCurrentMatch->second);
    if (!match.getWhitePlayer().has_value() || !match.getBluePlayer().has_value())
        return;

    const auto &ruleset = category.getRuleset();

    auto masterTime = mStoreManager.masterTime();
    if (ruleset.shouldAwardOsaekomiWazari(match, masterTime))
        mStoreManager.dispatch(std::make_unique<AwardWazariAction>(mCurrentMatch->first, mCurrentMatch->second, match.getOsaekomi()->first, masterTime, true));

    if (ruleset.shouldAwardOsaekomiIppon(match, masterTime))
        mStoreManager.dispatch(std::make_unique<AwardIpponAction>(mCurrentMatch->first, mCurrentMatch->second, match.getOsaekomi()->first, masterTime, true));

    if (ruleset.shouldStopOsaekomi(match, masterTime))
        mStoreManager.dispatch(std::make_unique<StopOsaekomiAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));

    if (ruleset.shouldPause(match, masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));
}

void ScoreOperatorWindow::changeNetworkClientState(NetworkClientState state) {
    mConnectAction->setEnabled(state == NetworkClientState::NOT_CONNECTED);
    mDisconnectAction->setEnabled(state == NetworkClientState::CONNECTED);
}

void ScoreOperatorWindow::osaekomiButtonClick(MatchStore::PlayerIndex playerIndex) {
    if (!mCurrentMatch)
        return;

    const auto &tournament = mStoreManager.getTournament();
    if (!tournament.containsCategory(mCurrentMatch->first))
        return;
    const auto &category = tournament.getCategory(mCurrentMatch->first);
    if (!category.containsMatch(mCurrentMatch->second))
        return;
    const auto &match = category.getMatch(mCurrentMatch->second);
    if (!match.getWhitePlayer().has_value() || !match.getBluePlayer().has_value())
        return;

    const auto &ruleset = category.getRuleset();

    auto masterTime = mStoreManager.masterTime();
    auto osaekomi = match.getOsaekomi();
    if (!osaekomi.has_value() || osaekomi->first != playerIndex) {
        // Start osaekomi
        mStoreManager.dispatch(std::make_unique<StartOsaekomiAction>(mCurrentMatch->first, mCurrentMatch->second, playerIndex, masterTime));

        if (match.getStatus() != MatchStatus::UNPAUSED && (match.getDuration() < ruleset.getNormalTime() || match.isGoldenScore()))
            mStoreManager.dispatch(std::make_unique<ResumeMatchAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));

    }
    else {
        // Stop osaekomi
        mStoreManager.dispatch(std::make_unique<StopOsaekomiAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));

        if (ruleset.shouldPause(match, masterTime))
            mStoreManager.dispatch(std::make_unique<PauseMatchAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));
    }
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

