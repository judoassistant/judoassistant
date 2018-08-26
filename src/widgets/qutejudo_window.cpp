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
#include "serialize.hpp"

#include "stores/category_store.hpp"
#include "stores/match_store.hpp"
#include "stores/match_event.hpp"
#include "stores/player_store.hpp"
#include "stores/qtournament_store.hpp"

#include "actions/actions.hpp"
#include "rulesets/rulesets.hpp"

QutejudoWindow::QutejudoWindow() {
    mTournament = std::make_unique<QTournamentStore>();

    createTournamentMenu();
    createEditMenu();
    createViewMenu();
    createPreferencesMenu();
    createHelpMenu();

    QTabWidget * tabWidget = new QTabWidget(this);
    tabWidget->addTab(new TournamentWidget(mTournament), tr("Tournament"));
    tabWidget->addTab(new PlayersWidget(mTournament), tr("Players"));
    tabWidget->addTab(new CategoriesWidget(mTournament), tr("Categories"));
    tabWidget->addTab(new TatamisWidget(mTournament), tr("Tatamis"));
    tabWidget->addTab(new MatchesWidget(mTournament), tr("Matches"));
    tabWidget->setCurrentIndex(1);
    tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    setCentralWidget(tabWidget);

    resize(250,250);
    setWindowTitle(tr("Qutejudo"));
}

void QutejudoWindow::createTournamentMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Tournament"));

    {
        QAction *action = new QAction(tr("New"), this);
        action->setShortcuts(QKeySequence::New);
        action->setToolTip(tr("Create a new tournament"));
        connect(action, &QAction::triggered, this, &QutejudoWindow::newTournament);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Open.."), this);
        action->setShortcuts(QKeySequence::Open);
        action->setToolTip(tr("Load tournament from a file"));
        connect(action, &QAction::triggered, this, &QutejudoWindow::openTournament);
        menu->addAction(action);
    }

    {
        QMenu * submenu = menu->addMenu("Open Recent");
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Save"), this);
        action->setShortcuts(QKeySequence::Save);
        action->setToolTip(tr("Save tournament to a file"));
        connect(action, &QAction::triggered, this, &QutejudoWindow::saveTournament);
        menu->addAction(action);
    }

    {
        QAction *action = new QAction(tr("Save As.."), this);
        action->setShortcuts(QKeySequence::SaveAs);
        action->setToolTip(tr("Save the tournament to file"));
        connect(action, &QAction::triggered, this, &QutejudoWindow::saveAsTournament);
        menu->addAction(action);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Quit"), this);
        action->setShortcuts(QKeySequence::Quit);
        action->setToolTip(tr("Quit Qutejudo"));
        menu->addAction(action);
    }
}

void QutejudoWindow::createEditMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Edit"));

    {
        QAction *action = new QAction(tr("Undo"), this);
        action->setShortcuts(QKeySequence::Undo);
        action->setToolTip(tr("Undo last action"));
        menu->addAction(action);
    }

    {
        QAction *action = new QAction(tr("Redo"), this);
        action->setShortcuts(QKeySequence::Redo);
        action->setToolTip(tr("Redo the last undone action"));
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

void QutejudoWindow::newTournament() {
    // TODO: implement
}

void QutejudoWindow::openTournament() {
    // TODO: Implement
}

void QutejudoWindow::writeTournament() {
    std::ofstream file(mFileName.toStdString(), std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        QMessageBox::information(this, tr("Unable to open file"), tr("The selected file could not be opened."));
        return;
    }

    cereal::PortableBinaryOutputArchive archive(file);
    archive(mTournament);
}
void QutejudoWindow::saveTournament() {
    if (mFileName.isEmpty())
        saveAsTournament();
    else
        writeTournament();
}

void QutejudoWindow::saveAsTournament() {
    qDebug() << QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Tournament"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Tournament Files (*.qj);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    mFileName = fileName;
    writeTournament();
}
