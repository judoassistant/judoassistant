#include <QMenu>
#include <QMenuBar>
#include <QTabWidget>
#include <QAction>
#include <QDesktopServices>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QStatusBar>
#include <QCoreApplication>
#include <QUrl>

#include <fstream>

#include "widgets/hub_window.hpp"
#include "widgets/tournament_widget.hpp"
#include "widgets/players_widget.hpp"
#include "widgets/categories_widget.hpp"
#include "widgets/tatamis_widget.hpp"
#include "widgets/matches_widget.hpp"
#include "widgets/import_players_csv_dialog.hpp"
#include "widgets/import_helpers/csv_reader.hpp"
#include "config/web.hpp"
#include "exception.hpp"

HubWindow::HubWindow() {
    // TODO: Add todostack sidebar
    // TODO: Catch quit signal to quit gracefully 
    createStatusBar();
    createTournamentMenu();
    createEditMenu();
    createViewMenu();
    createPreferencesMenu();
    createHelpMenu();

    QTabWidget * tabWidget = new QTabWidget(this);
    tabWidget->addTab(new TournamentWidget(mStoreManager), tr("Tournament"));
    tabWidget->addTab(new PlayersWidget(mStoreManager), tr("Players"));
    tabWidget->addTab(new CategoriesWidget(mStoreManager), tr("Categories"));
    tabWidget->addTab(new TatamisWidget(mStoreManager), tr("Tatamis"));
    tabWidget->addTab(new MatchesWidget(mStoreManager), tr("Matches"));
    tabWidget->setCurrentIndex(1);
    tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    setCentralWidget(tabWidget);

    resize(250,250);
    setWindowTitle(tr("JudoAssistant"));

    mStoreManager.startServer(8000);
}

void HubWindow::createStatusBar() {
    statusBar();
}

void HubWindow::createTournamentMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Tournament"));

    {
        QAction *action = new QAction(tr("New"), this);
        action->setShortcuts(QKeySequence::New);
        action->setStatusTip(tr("Create a new tournament"));
        connect(action, &QAction::triggered, this, &HubWindow::newTournament);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Open.."), this);
        action->setShortcuts(QKeySequence::Open);
        action->setStatusTip(tr("Load tournament from a file"));
        connect(action, &QAction::triggered, this, &HubWindow::openTournament);
        menu->addAction(action);
    }

    {
        // TODO: Implement open recent
        // QMenu * submenu = menu->addMenu("Open Recent");
    }

    {
        QMenu *submenu = menu->addMenu(tr("Import.."));

        {
            QAction *action = new QAction(tr("Import players.."), this);
            action->setStatusTip(tr("Import players from a file"));
            connect(action, &QAction::triggered, this, &HubWindow::openImportPlayers);
            submenu->addAction(action);
        }
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Save"), this);
        action->setShortcuts(QKeySequence::Save);
        action->setStatusTip(tr("Save tournament to a file"));
        connect(action, &QAction::triggered, this, &HubWindow::saveTournament);
        menu->addAction(action);
    }

    {
        QAction *action = new QAction(tr("Save As.."), this);
        action->setShortcuts(QKeySequence::SaveAs);
        action->setStatusTip(tr("Save the tournament to file"));
        connect(action, &QAction::triggered, this, &HubWindow::saveAsTournament);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Quit"), this);
        action->setShortcuts(QKeySequence::Quit);
        action->setStatusTip(tr("Quit JudoAssistant"));
        connect(action, &QAction::triggered, this, &HubWindow::quit);
        menu->addAction(action);
    }
}

void HubWindow::createEditMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Edit"));

    {
        QAction *action = new QAction(tr("Undo"), this);
        action->setShortcuts(QKeySequence::Undo);
        action->setStatusTip(tr("Undo last action"));
        action->setEnabled(mStoreManager.canUndo());
        menu->addAction(action);
        connect(&mStoreManager, &MasterStoreManager::undoStatusChanged, action, &QAction::setEnabled);
        connect(action, &QAction::triggered, &mStoreManager, &MasterStoreManager::undo);
    }

    {
        QAction *action = new QAction(tr("Redo"), this);
        action->setShortcuts(QKeySequence::Redo);
        action->setStatusTip(tr("Redo the last undone action"));
        action->setEnabled(mStoreManager.canRedo());
        menu->addAction(action);
        connect(&mStoreManager, &MasterStoreManager::redoStatusChanged, action, &QAction::setEnabled);
        connect(action, &QAction::triggered, &mStoreManager, &MasterStoreManager::redo);
    }
}

void HubWindow::createViewMenu() {
    // QMenu *menu = menuBar()->addMenu(tr("View"));
}

void HubWindow::createPreferencesMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Preferences"));
    {
        QMenu *submenu = menu->addMenu("Language");
        QAction *englishAction = new QAction(tr("English"), this);
        submenu->addAction(englishAction);
    }
}

void HubWindow::createHelpMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Help"));
    {
        QAction *action = new QAction(tr("JudoAssistant Home Page"), this);
        connect(action, &QAction::triggered, this, &HubWindow::openHomePage);
        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("JudoAssistant Manual"), this);
        connect(action, &QAction::triggered, this, &HubWindow::openManual);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Report an Issue"), this);
        connect(action, &QAction::triggered, this, &HubWindow::openReportIssue);

        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("About"), this);
        connect(action, &QAction::triggered, this, &HubWindow::showAboutDialog);
        menu->addAction(action);
    }
}

void HubWindow::openHomePage() {
    if(QDesktopServices::openUrl(Config::HOME_PAGE_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open home page."));
}

void HubWindow::openManual() {
    if(QDesktopServices::openUrl(Config::MANUAL_URL))
        return;

    QMessageBox::warning(this, tr("Unable to open manual."), tr("The manual is available at"));
}

void HubWindow::openReportIssue() {
    if(QDesktopServices::openUrl(Config::REPORT_ISSUE_URL))
        return;

    // QMessageBox::warning(this, tr("Unable to open report issue page."));
}

void HubWindow::writeTournament() {
    if (!mStoreManager.write(mFileName))
        QMessageBox::warning(this, tr("Unable to open file"), tr("The selected file could not be opened."));
    else
        statusBar()->showMessage(tr("Saved tournament to file"));
}

void HubWindow::readTournament() {
    readTournament(mFileName);
}

void HubWindow::readTournament(const QString &fileName) {
    mFileName = fileName;
    if (!mStoreManager.read(fileName))
        QMessageBox::warning(this, tr("Unable to open file"), tr("The selected file could not be opened."));
    else
        statusBar()->showMessage(tr("Opened tournament from file"));
}

void HubWindow::newTournament() {
    if (mStoreManager.isDirty()) {
        auto reply = QMessageBox::question(this, tr("Unsaved changes"), tr("The current tournament has unsaved changes. Would you still like to continue?"), QMessageBox::Yes | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel)
            return;
    }

    mFileName = "";
    mStoreManager.resetTournament();
}

void HubWindow::openTournament() {
    if (mStoreManager.isDirty()) {
        auto reply = QMessageBox::question(this, tr("Unsaved changes"), tr("The current tournament has unsaved changes. Would you still like to continue?"), QMessageBox::Yes | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel)
            return;
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Tournament"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Tournament Files (*.qj);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    readTournament(fileName);
}

void HubWindow::quit() {
    if (mStoreManager.isDirty()) {
        auto reply = QMessageBox::question(this, tr("Unsaved changes"), tr("The current tournament has unsaved changes. Would you still like to exit without saving?"), QMessageBox::Yes | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel)
            return;
    }

    QCoreApplication::exit();
}

void HubWindow::saveTournament() {
    if (mFileName.isEmpty())
        saveAsTournament();
    else
        writeTournament();
}

void HubWindow::saveAsTournament() {
    // TODO: Append .qj if not already
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Tournament"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Tournament Files (*.qj);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    mFileName = fileName;
    writeTournament();
}

void HubWindow::showAboutDialog() {
    QMessageBox::about(this, tr("JudoAssistant - About"), tr("TODO"));
}

void HubWindow::openImportPlayers() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import players"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Comma-separated (CSV) Files (*.csv);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    CSVReader reader(fileName);
    if (!reader.isOpen()) {
        QMessageBox::warning(this, tr("Unable to open file"), tr("The given file could not be opened."));
        return;
    }


    ImportPlayersCSVDialog dialog(mStoreManager, &reader);

    dialog.exec();
}

