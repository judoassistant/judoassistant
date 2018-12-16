#pragma once

#include <QMainWindow>

#include "core.hpp"
#include "store_managers/store_manager.hpp"

class ScoreDisplayWidget;

class ScoreDisplayWindow : public QMainWindow {
    Q_OBJECT

public:
    ScoreDisplayWindow(const StoreManager &storeManager);

private slots:
private:
    const StoreManager &mStoreManager;
    ScoreDisplayWidget *mScoreWidget;
};

