#pragma once

#include <QMainWindow>
#include <QString>

#include "core.hpp"
#include "store_managers/store_manager.hpp"

class MatchesWidget : public QWidget {
    Q_OBJECT

public:
    MatchesWidget(StoreManager &storeManager);
};

