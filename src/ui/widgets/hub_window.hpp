#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QMenu>

#include "core/core.hpp"
#include "ui/store_managers/master_store_manager.hpp"

class HubWindow : public QMainWindow {
    Q_OBJECT

public:
    HubWindow();

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
    void showTournamentPreferences();
    void showJudoAssistantPreferences();
    void showServerStartFailure();
    void autosaveTimerHit();

private:
    void createStatusBar();
    void createTournamentMenu();
    void createEditMenu();
    void createViewMenu();
    void createPreferencesMenu();
    void createHelpMenu();

    void refreshRecentFilesMenu();
    void addToRecentFiles(const QString &file);
    void createActionForFile(const QString &file);

    void writeTournament();
    MasterStoreManager mStoreManager;
    QString mFileName;

    QMenu *mRecentFilesMenu;

    QTimer mAutosaveTimer;
    std::optional<std::chrono::seconds> mAutosaveFrequency;
};

