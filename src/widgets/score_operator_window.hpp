#pragma once

#include <QMainWindow>

#include "core.hpp"
#include "store_managers/client_store_manager.hpp"

class ScoreOperatorWindow : public QMainWindow {
    Q_OBJECT

public:
    ScoreOperatorWindow();

private slots:
    void quit();
    void openHomePage();
    void openManual();
    void openReportIssue();
    void showAboutDialog();

private:
    void createStatusBar();
    void createTournamentMenu();
    void createEditMenu();
    void createViewMenu();
    void createPreferencesMenu();
    void createHelpMenu();

    void showConnectDialog();

    ClientStoreManager mStoreManager;
};

