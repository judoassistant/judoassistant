#pragma once

#include <QMainWindow>

#include "core.hpp"
#include "store_managers/master_store_manager.hpp"

class HubWindow : public QMainWindow {
    Q_OBJECT

public:
    HubWindow();

    void readTournament();
    void readTournament(const QString &fileName);
    void startServer();

private slots:
    void quit();
    void newTournament();
    void openTournament();
    void openImportPlayers();
    void saveTournament();
    void saveAsTournament();
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

    void writeTournament();
    MasterStoreManager mStoreManager;
    QString mFileName;
};

