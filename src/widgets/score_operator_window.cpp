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

#include "config/web.hpp"
#include "widgets/score_display_widget.hpp"
#include "widgets/score_operator_window.hpp"
#include "widgets/connect_dialog.hpp"
#include "widgets/models/actions_model.hpp"

ScoreOperatorWindow::ScoreOperatorWindow() {
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
    log_debug().msg("Dialog accepted");
}

QWidget* ScoreOperatorWindow::createMainArea() {
    QWidget *res = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(res);

    {
        QGroupBox *viewBox = new QGroupBox("Spectator View", res);
        QVBoxLayout *subLayout = new QVBoxLayout(res);

        ScoreDisplayWidget *displayWidget = new ScoreDisplayWidget(viewBox);
        subLayout->addWidget(displayWidget);

        viewBox->setLayout(subLayout);
        layout->addWidget(viewBox);
    }

    QGroupBox *whiteBox = new QGroupBox("White Player Controls", res);
    QGroupBox *blueBox = new QGroupBox("Blue Player Controls", res);
    QGroupBox *matchBox = new QGroupBox("Match Controls", res);

    layout->addWidget(whiteBox);
    layout->addWidget(blueBox);
    layout->addWidget(matchBox);

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



    QGroupBox *nextMatchBox = new QGroupBox("Next Match", res);

    layout->addWidget(nextMatchBox);

    res->setLayout(layout);
    return res;
}

