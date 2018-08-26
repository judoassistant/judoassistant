#include <QMenu>
#include <QMenuBar>
#include <QTabWidget>
#include <QAction>
#include <QDesktopServices>
#include <QFileDialog>
#include <QtDebug>
#include <QStandardPaths>
#include <QMessageBox>

#include <fstream>

#include "widgets/qutejudo_window.hpp"
#include "widgets/tournament_widget.hpp"
#include "widgets/players_widget.hpp"
#include "widgets/categories_widget.hpp"
#include "widgets/tatamis_widget.hpp"
#include "widgets/matches_widget.hpp"
#include "config/web.hpp"
#include "exception.hpp"

QutejudoWindow::QutejudoWindow() {
    createStatusBar();
    createTournamentMenu();
    createEditMenu();
    createViewMenu();
    createPreferencesMenu();
    createHelpMenu();

    QTabWidget * tabWidget = new QTabWidget(this);
    tabWidget->addTab(new TournamentWidget(mStoreHandler), tr("Tournament"));
    tabWidget->addTab(new PlayersWidget(mStoreHandler), tr("Players"));
    tabWidget->addTab(new CategoriesWidget(mStoreHandler), tr("Categories"));
    tabWidget->addTab(new TatamisWidget(mStoreHandler), tr("Tatamis"));
    tabWidget->addTab(new MatchesWidget(mStoreHandler), tr("Matches"));
    tabWidget->setCurrentIndex(1);
    tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    setCentralWidget(tabWidget);

    resize(250,250);
    setWindowTitle(tr("Qutejudo"));
}

void QutejudoWindow::createStatusBar() {
    statusBar();
}

void QutejudoWindow::createTournamentMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Tournament"));

    {
        QAction *action = new QAction(tr("New"), this);
        action->setShortcuts(QKeySequence::New);
        action->setStatusTip(tr("Create a new tournament"));
        connect(action, &QAction::triggered, this, &QutejudoWindow::newTournament);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Open.."), this);
        action->setShortcuts(QKeySequence::Open);
        action->setStatusTip(tr("Load tournament from a file"));
        connect(action, &QAction::triggered, this, &QutejudoWindow::openTournament);
        menu->addAction(action);
    }

    {
        // TODO: Implement open recent
        // QMenu * submenu = menu->addMenu("Open Recent");
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Save"), this);
        action->setShortcuts(QKeySequence::Save);
        action->setStatusTip(tr("Save tournament to a file"));
        connect(action, &QAction::triggered, this, &QutejudoWindow::saveTournament);
        menu->addAction(action);
    }

    {
        QAction *action = new QAction(tr("Save As.."), this);
        action->setShortcuts(QKeySequence::SaveAs);
        action->setStatusTip(tr("Save the tournament to file"));
        connect(action, &QAction::triggered, this, &QutejudoWindow::saveAsTournament);
        menu->addAction(action);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Quit"), this);
        action->setShortcuts(QKeySequence::Quit);
        action->setStatusTip(tr("Quit Qutejudo"));
        menu->addAction(action);
    }
}

void QutejudoWindow::createEditMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Edit"));

    {
        QAction *action = new QAction(tr("Undo"), this);
        action->setShortcuts(QKeySequence::Undo);
        action->setStatusTip(tr("Undo last action"));
        menu->addAction(action);
    }

    {
        QAction *action = new QAction(tr("Redo"), this);
        action->setShortcuts(QKeySequence::Redo);
        action->setStatusTip(tr("Redo the last undone action"));
        menu->addAction(action);
    }
}

void QutejudoWindow::createViewMenu() {
    QMenu *menu = menuBar()->addMenu(tr("View"));
}

void QutejudoWindow::createPreferencesMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Preferences"));
    {
        QMenu *submenu = menu->addMenu("Language");
        QAction *englishAction = new QAction(tr("English"), this);
        submenu->addAction(englishAction);
    }
}

void QutejudoWindow::createHelpMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Help"));
    {
        QAction *action = new QAction(tr("Qutejudo Home Page"), this);
        connect(action, &QAction::triggered, this, &QutejudoWindow::openHomePage);
        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("Qutejudo Manual"), this);
        connect(action, &QAction::triggered, this, &QutejudoWindow::openManual);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Report an Issue"), this);
        connect(action, &QAction::triggered, this, &QutejudoWindow::openReportIssue);

        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("About"), this);
        menu->addAction(action);
    }
}

void QutejudoWindow::openHomePage() {
    if(QDesktopServices::openUrl(Config::HOME_PAGE_URL))
        return;

    // QMessageBox::information(this, tr("Unable to open home page."));
}

void QutejudoWindow::openManual() {
    if(QDesktopServices::openUrl(Config::MANUAL_URL))
        return;

    QMessageBox::information(this, tr("Unable to open manual."), tr("The manual is available at"));
}

void QutejudoWindow::openReportIssue() {
    if(QDesktopServices::openUrl(Config::REPORT_ISSUE_URL))
        return;

    // QMessageBox::information(this, tr("Unable to open report issue page."));
}

void QutejudoWindow::writeTournament() {
    if (!mStoreHandler.write(mFileName))
        QMessageBox::information(this, tr("Unable to open file"), tr("The selected file could not be opened."));
    else
        statusBar()->showMessage(tr("Saved tournament to file"));
}

void QutejudoWindow::readTournament() {
    if (!mStoreHandler.read(mFileName))
        QMessageBox::information(this, tr("Unable to open file"), tr("The selected file could not be opened."));
    else
        statusBar()->showMessage(tr("Opened tournament from file"));
}

void QutejudoWindow::newTournament() {
    // TODO: handle unsaved changes
    mFileName = "";
    mStoreHandler.reset();
}

void QutejudoWindow::openTournament() {
    // TODO: handle unsaved changes
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Tournament"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Tournament Files (*.qj);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    mFileName = fileName;
    readTournament();
}

void QutejudoWindow::saveTournament() {
    if (mFileName.isEmpty())
        saveAsTournament();
    else
        writeTournament();
}

void QutejudoWindow::saveAsTournament() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Tournament"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Tournament Files (*.qj);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    mFileName = fileName;
    writeTournament();
}
