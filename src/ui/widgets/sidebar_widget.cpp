#include "ui/widgets/sidebar_widget.hpp"

SidebarWidget::SidebarWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setTabPosition(QTabWidget::TabPosition::West);
}
