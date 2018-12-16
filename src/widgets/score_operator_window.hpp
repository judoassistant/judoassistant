#pragma once

#include <stack>

#include <QMainWindow>
#include <QMetaObject>

#include "core.hpp"
#include "store_managers/client_store_manager.hpp"

class ScoreOperatorWindow : public QMainWindow {
    Q_OBJECT

public:
    ScoreOperatorWindow();

private:
    void quit();
    void openHomePage();
    void openManual();
    void openReportIssue();
    void showAboutDialog();

    void clearTatamiMenu();
    void populateTatamiMenu();

    void beginResetTournament();
    void endResetTournament();

    std::optional<std::pair<CategoryId, MatchId>> getNextMatch();

    void createStatusBar();
    void createTournamentMenu();
    void createEditMenu();
    void createViewMenu();
    void createPreferencesMenu();
    void createHelpMenu();

    QWidget* createMainArea();
    QWidget* createSideArea();

    void showConnectDialog();

    ClientStoreManager mStoreManager;
    QMenu *mTatamiMenu;
    int mTatami;
    std::optional<std::pair<CategoryId, MatchId>> mCurrentMatch;
    std::stack<QMetaObject::Connection> mConnections;
};

