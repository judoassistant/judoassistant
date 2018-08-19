#include <QMenu>
#include <QMenuBar>
#include <QTabWidget>
#include <QAction>

#include "widgets/qutejudo_window.hpp"
#include "widgets/tournament_widget.hpp"
#include "widgets/players_widget.hpp"
#include "widgets/categories_widget.hpp"
#include "widgets/tatamis_widget.hpp"
#include "widgets/matches_widget.hpp"

QutejudoWindow::QutejudoWindow() {
    createTournamentMenu();
    createEditMenu();
    createViewMenu();
    createPreferencesMenu();
    createHelpMenu();

    QTabWidget * tabWidget = new QTabWidget(this);
    tabWidget->addTab(new TournamentWidget(), tr("Tournament"));
    tabWidget->addTab(new PlayersWidget(), tr("Players"));
    tabWidget->addTab(new CategoriesWidget(), tr("Categories"));
    tabWidget->addTab(new TatamisWidget(), tr("Tatamis"));
    tabWidget->addTab(new MatchesWidget(), tr("Matches"));
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
        action->setStatusTip(tr("Create a new tournament"));
        connect(action, &QAction::triggered, this, &QutejudoWindow::newTournament);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Open.."), this);
        action->setShortcuts(QKeySequence::Open);
        action->setStatusTip(tr("Open an existing tournament file"));
        menu->addAction(action);
    }

    {
        QMenu * submenu = menu->addMenu("Open Recent");
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Save"), this);
        action->setShortcuts(QKeySequence::Save);
        action->setStatusTip(tr("Save the tournament to file"));
        menu->addAction(action);
    }

    {
        QAction *action = new QAction(tr("Save As.."), this);
        action->setShortcuts(QKeySequence::SaveAs);
        action->setStatusTip(tr("Save the tournament to file"));
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

void QutejudoWindow::newTournament() {

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
        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("Qutejudo Manual"), this);
        menu->addAction(action);
    }

    menu->addSeparator();

    {
        QAction *action = new QAction(tr("Report an Issue"), this);
        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("About"), this);
        menu->addAction(action);
    }
}

