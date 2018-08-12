#include "widgets/qutejudo_widget.hpp"
#include "widgets/players_widget.hpp"
#include "widgets/categories_widget.hpp"

QutejudoWidget::QutejudoWidget() {
    m_tabWidget = new QTabWidget();
    m_tabWidget->addTab(new PlayersWidget(), "Players");
    m_tabWidget->addTab(new CategoriesWidget(), "Categories");
    m_tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    setCentralWidget(m_tabWidget);

    resize(250,250);
    setWindowTitle("Qutejudo");
}

QutejudoWidget::~QutejudoWidget() {
    delete m_tabWidget;
}
