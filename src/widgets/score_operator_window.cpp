#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>

#include "actions/match_actions.hpp"
#include "config/web.hpp"
#include "stores/category_store.hpp"
#include "stores/match_store.hpp"
#include "stores/qtournament_store.hpp"
#include "widgets/score_operator_window.hpp"
#include "widgets/connect_dialog.hpp"
#include "widgets/models/actions_model.hpp"

ScoreOperatorWindow::ScoreOperatorWindow()
    : mTatami(-1)
{
    createStatusBar();
    createTournamentMenu();
    createEditMenu();
    createViewMenu();
    createPreferencesMenu();
    createHelpMenu();

    QSplitter * splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);

    splitter->addWidget(createMainArea());
    splitter->addWidget(createSideArea());

    setCentralWidget(splitter);

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

    {
        QAction *action = new QAction(tr("Connect.."), this);
        action->setShortcuts(QKeySequence::New);
        action->setStatusTip(tr("Connect to hub"));
        connect(action, &QAction::triggered, this, &ScoreOperatorWindow::showConnectDialog);
        menu->addAction(action);
    }

    // menu->addSeparator();
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
    for (int i = 0; i < static_cast<int>(tatamis.tatamiCount()); ++i) {
        QAction *action = new QAction(tr("Tatami %1").arg(QString::number(i+1)), mTatamiMenu);
        action->setCheckable(true);
        mTatamiActionGroup->addAction(action);
        if (i == mTatami) {
            action->setChecked(true);
        }

        connect(action, &QAction::triggered, this, [=]() { setTatami(i); });

        mTatamiMenu->addAction(action);
    }
}

void ScoreOperatorWindow::createViewMenu() {
    // QMenu *menu = menuBar()->addMenu(tr("View"));
}

void ScoreOperatorWindow::createPreferencesMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Preferences"));
    {
        QMenu *submenu = menu->addMenu("Language");
        QAction *englishAction = new QAction(tr("English"), this);
        submenu->addAction(englishAction);
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
    if(QDesktopServices::openUrl(Config::HOME_PAGE_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open home page."));
}

void ScoreOperatorWindow::openManual() {
    if(QDesktopServices::openUrl(Config::MANUAL_URL))
        return;

    QMessageBox::warning(this, tr("Unable to open manual."), tr("The manual is available at"));
}

void ScoreOperatorWindow::openReportIssue() {
    if(QDesktopServices::openUrl(Config::REPORT_ISSUE_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open report issue page."));
}

void ScoreOperatorWindow::quit() {
    // TODO: disconnect
    QCoreApplication::exit();
}

void ScoreOperatorWindow::showAboutDialog() {
    QMessageBox::about(this, tr("JudoAssistant - About"), tr("TODO"));
}

void ScoreOperatorWindow::showConnectDialog() {
    ConnectDialog dialog(mStoreManager);
    dialog.exec();
}

QWidget* ScoreOperatorWindow::createMainArea() {
    QWidget *res = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(res);

    {
        QGroupBox *viewBox = new QGroupBox("Spectator View", res);
        QVBoxLayout *subLayout = new QVBoxLayout(res);

        mScoreDisplayWidget = new ScoreDisplayWidget(mStoreManager, viewBox);
        subLayout->addWidget(mScoreDisplayWidget);

        viewBox->setLayout(subLayout);
        layout->addWidget(viewBox);
    }

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

        subLayout->addWidget(mWhiteIpponButton);
        subLayout->addWidget(mWhiteWazariButton);
        subLayout->addWidget(mWhiteShidoButton);
        subLayout->addWidget(mWhiteHansokuMakeButton);

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

        subLayout->addWidget(mBlueIpponButton);
        subLayout->addWidget(mBlueWazariButton);
        subLayout->addWidget(mBlueShidoButton);
        subLayout->addWidget(mBlueHansokuMakeButton);

        blueBox->setLayout(subLayout);
        layout->addWidget(blueBox);
    }

    {
        QGroupBox *matchBox = new QGroupBox("Match Controls", res);
        QVBoxLayout *subLayout = new QVBoxLayout(res);

        mNextButton = new QPushButton("Go to next match", matchBox);
        connect(mNextButton, &QPushButton::clicked, this, &ScoreOperatorWindow::goNextMatch);
        mResumeButton = new QPushButton("Resume Match", matchBox);
        connect(mResumeButton, &QPushButton::clicked, this, &ScoreOperatorWindow::resumeButtonClick);

        subLayout->addWidget(mNextButton);
        subLayout->addWidget(mResumeButton);

        matchBox->setLayout(subLayout);
        layout->addWidget(matchBox);
    }

    res->setLayout(layout);
    return res;
}

QWidget* ScoreOperatorWindow::createSideArea() {
    QWidget *res = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(res);

    {
        auto model = new ActionsProxyModel(mStoreManager, res);

        QGroupBox *actionBox = new QGroupBox("Actions", res);
        QVBoxLayout *subLayout = new QVBoxLayout(actionBox);

        auto tableView = new QTableView(actionBox);
        tableView->setModel(model);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->horizontalHeader()->setStretchLastSection(true);
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        // tableView->setSortingEnabled(false);
        // tableView->sortByColumn(1, Qt::AscendingOrder);
        // tableView->setContextMenuPolicy(Qt::CustomContextMenu);

        subLayout->addWidget(tableView);

        actionBox->setLayout(subLayout);

        layout->addWidget(actionBox);
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

void ScoreOperatorWindow::beginResetTournament() {
    while (!mConnections.empty()) {
        disconnect(mConnections.top());
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

void ScoreOperatorWindow::setTatami(int tatami) {
    mTatami = tatami;

    findNextMatch();
}

void ScoreOperatorWindow::silentConnect(QString host, int port) {
    mStoreManager.connect(host, port);
}

void ScoreOperatorWindow::changeTatamis(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {
    for (const auto &location: locations) {
        if (static_cast<int>(location.tatamiIndex) == mTatami) {
            findNextMatch();
            return;
        }
    }
}

void ScoreOperatorWindow::findNextMatch() {
    log_debug().msg("Finding next match");
    auto &tournament = mStoreManager.getTournament();
    auto &tatamis = tournament.getTatamis();

    if (mTatami != -1 && mTatami < static_cast<int>(tatamis.tatamiCount())) {
        auto &tatami = tatamis[mTatami];

        for (size_t i = 0; i < tatami.groupCount(); ++i) {
            const auto &group = tatami.getGroup(i);
            if (group.getStatus() == ConcurrentBlockGroup::Status::FINISHED) continue;

            for (auto combinedId : group.getMatches()) {
                if (mCurrentMatch == combinedId) {
                    continue;
                }

                const auto &match = tournament.getCategory(combinedId.first).getMatch(combinedId.second);

                if (match.getStatus() != MatchStatus::FINISHED) {
                    log_debug().field("combinedId", combinedId).msg("Found match");
                    mNextMatch = combinedId;
                    mNextMatchWidget->setMatch(combinedId);
                    updateNextButton();
                    return;
                }
            }

            assert(false); // should never reach this
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

void ScoreOperatorWindow::beginResetMatches(CategoryId categoryId) {
    // next match category resetting is handled by tatami hooks
    if (mCurrentMatch.has_value() && mCurrentMatch->first == categoryId) {
        disableControlButtons(); // disable controls
    }
}

void ScoreOperatorWindow::endResetMatches(CategoryId categoryId) {
    // next match category resetting is handled by tatami hooks
    updateControlButtons();
}

void ScoreOperatorWindow::updateNextButton() {
    bool enabled = true;
    if (!mNextMatch.has_value()) {
        enabled = false;
    }
    else {
        const auto &match = mStoreManager.getTournament().getCategory(mNextMatch->first).getMatch(mNextMatch->second);
        if (!match.getWhitePlayer() || !match.getBluePlayer())
            enabled = false;
    }

    if (enabled && mCurrentMatch.has_value()) {
        const auto &match = mStoreManager.getTournament().getCategory(mNextMatch->first).getMatch(mNextMatch->second);
        if (match.getStatus() != MatchStatus::FINISHED)
            enabled = false;
    }

    mNextButton->setEnabled(enabled);
}

void ScoreOperatorWindow::goNextMatch() {
    if (!mNextButton->isEnabled()) return;

    mCurrentMatch = mNextMatch;
    mScoreDisplayWidget->setMatch(mCurrentMatch);
    findNextMatch();
    updateControlButtons();
}

void ScoreOperatorWindow::disableControlButtons() {
    mResumeButton->setEnabled(false);
    mResumeButton->setText("Resume Match");

    mWhiteIpponButton->setEnabled(false);
    mWhiteWazariButton->setEnabled(false);
    mWhiteShidoButton->setEnabled(false);
    mWhiteHansokuMakeButton->setEnabled(false);

    mBlueIpponButton->setEnabled(false);
    mBlueWazariButton->setEnabled(false);
    mBlueShidoButton->setEnabled(false);
    mBlueHansokuMakeButton->setEnabled(false);
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

    if (match.getStatus() == MatchStatus::UNPAUSED) {
        mResumeButton->setText("Pause match");
        mResumeButton->setEnabled(true);
    }
    else {
        mResumeButton->setText("Resume Match");
        mResumeButton->setEnabled(ruleset.canResume(match, mStoreManager.masterTime()));
    }

    mWhiteIpponButton->setEnabled(ruleset.canAddIppon(match, MatchStore::PlayerIndex::WHITE));
    mWhiteWazariButton->setEnabled(ruleset.canAddWazari(match, MatchStore::PlayerIndex::WHITE));
    mWhiteShidoButton->setEnabled(ruleset.canAddShido(match, MatchStore::PlayerIndex::WHITE));
    mWhiteHansokuMakeButton->setEnabled(ruleset.canAddHansokuMake(match, MatchStore::PlayerIndex::WHITE));

    mBlueIpponButton->setEnabled(ruleset.canAddIppon(match, MatchStore::PlayerIndex::BLUE));
    mBlueWazariButton->setEnabled(ruleset.canAddWazari(match, MatchStore::PlayerIndex::BLUE));
    mBlueShidoButton->setEnabled(ruleset.canAddShido(match, MatchStore::PlayerIndex::BLUE));
    mBlueHansokuMakeButton->setEnabled(ruleset.canAddHansokuMake(match, MatchStore::PlayerIndex::BLUE));
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
        log_debug().msg("Pausing match");
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(mCurrentMatch->first, mCurrentMatch->second, mStoreManager.masterTime()));
    }
    else {
        auto masterTime = mStoreManager.masterTime();
        if (!ruleset.canResume(match, masterTime))
            return;
        log_debug().msg("Resuming match");
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
    mStoreManager.dispatch(std::make_unique<AwardIpponAction>(mCurrentMatch->first, mCurrentMatch->second, playerIndex, match.currentDuration(masterTime)));

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
    mStoreManager.dispatch(std::make_unique<AwardWazariAction>(mCurrentMatch->first, mCurrentMatch->second, playerIndex, match.currentDuration(masterTime)));

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
    mStoreManager.dispatch(std::make_unique<AwardShidoAction>(mCurrentMatch->first, mCurrentMatch->second, playerIndex, match.currentDuration(masterTime)));

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
    mStoreManager.dispatch(std::make_unique<AwardHansokuMakeAction>(mCurrentMatch->first, mCurrentMatch->second, playerIndex, match.currentDuration(masterTime)));

    if (ruleset.shouldPause(match, masterTime))
        mStoreManager.dispatch(std::make_unique<PauseMatchAction>(mCurrentMatch->first, mCurrentMatch->second, masterTime));
}

