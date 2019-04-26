#pragma once

#include <QMainWindow>

#include "core/core.hpp"
#include "ui/store_managers/store_manager.hpp"

class ScoreDisplayWidget;

class ScoreDisplayWindow : public QMainWindow {
    Q_OBJECT

public:
    ScoreDisplayWindow(const StoreManager &storeManager);

    ScoreDisplayWidget& getDisplayWidget();

private slots:
private:
    const StoreManager &mStoreManager;
    ScoreDisplayWidget *mScoreWidget;
};

