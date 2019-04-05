#pragma once

#include <QTabWidget>

#include "core/core.hpp"
#include "ui/store_managers/master_store_manager.hpp"

class SidebarWidget : public QTabWidget {
    Q_OBJECT

public:
    SidebarWidget(QWidget *parent = nullptr);
};

