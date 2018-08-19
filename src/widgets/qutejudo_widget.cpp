#include <QMenu>
#include <QMenuBar>
#include <QTabWidget>
#include <QAction>

#include "widgets/qutejudo_widget.hpp"
#include "widgets/tournament_widget.hpp"
#include "widgets/players_widget.hpp"
#include "widgets/categories_widget.hpp"
#include "widgets/tatamis_widget.hpp"
#include "widgets/matches_widget.hpp"

QutejudoWidget::QutejudoWidget() {
    createTournamentMenu();
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

void QutejudoWidget::createTournamentMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Tournament"));

    QAction *newAct = new QAction(tr("New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &QutejudoWidget::newFile);
    menu->addAction(newAct);
}

void QutejudoWidget::newFile() {
}

void QutejudoWidget::createViewMenu() {
    QMenu *menu = menuBar()->addMenu(tr("View"));
}

void QutejudoWidget::createPreferencesMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Preferences"));
    {
        QMenu *submenu = menu->addMenu("Language");
        QAction *englishAction = new QAction(tr("English"), this);
        submenu->addAction(englishAction);
    }
}

void QutejudoWidget::createHelpMenu() {
    QMenu *menu = menuBar()->addMenu(tr("Help"));
    {
        QAction *action = new QAction(tr("Qutejudo Home Page"), this);
        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("Qutejudo Manual"), this);
        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("Report an Issue"), this);
        menu->addAction(action);
    }
    {
        QAction *action = new QAction(tr("About"), this);
        menu->addAction(action);
    }
}

