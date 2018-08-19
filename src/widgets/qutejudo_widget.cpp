#include <QMenu>
#include <QMenuBar>

#include "widgets/qutejudo_widget.hpp"
#include "widgets/tournament_widget.hpp"
#include "widgets/players_widget.hpp"
#include "widgets/categories_widget.hpp"
#include "widgets/tatamis_widget.hpp"
#include "widgets/matches_widget.hpp"

QutejudoWidget::QutejudoWidget() {
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    QMenu *windowMenu = menuBar()->addMenu(tr("Window"));
    QMenu *preferencesMenu = menuBar()->addMenu(tr("Preferences"));
    QMenu *helpMenu = menuBar()->addMenu(tr("Help"));

    m_tabWidget = new QTabWidget();
    m_tabWidget->addTab(new TournamentWidget(), tr("Tournament"));
    m_tabWidget->addTab(new PlayersWidget(), tr("Players"));
    m_tabWidget->addTab(new CategoriesWidget(), tr("Categories"));
    m_tabWidget->addTab(new TatamisWidget(), tr("Tatamis"));
    m_tabWidget->addTab(new MatchesWidget(), tr("Matches"));
    m_tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    setCentralWidget(m_tabWidget);

    resize(250,250);
    setWindowTitle("Qutejudo");
}

QutejudoWidget::~QutejudoWidget() {
    delete m_tabWidget;
}
