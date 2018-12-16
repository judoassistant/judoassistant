#pragma once

#include <stack>

#include <QActionGroup>
#include <QMainWindow>
#include <QMetaObject>

#include "core.hpp"
#include "store_managers/client_store_manager.hpp"
#include "stores/tatami_store.hpp"
#include "widgets/match_card_widget.hpp"
#include "widgets/score_display_widget.hpp"

class ScoreOperatorWindow : public QMainWindow {
    Q_OBJECT

public:
    ScoreOperatorWindow();
    void silentConnect(QString host, int port=8000); // TODO: Setup default port in config

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

    void changeTatamis(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);

    void setTatami(int tatami);

    void findNextMatch();

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
    QActionGroup *mTatamiActionGroup;
    MatchCardWidget *mNextMatchWidget;
    ScoreDisplayWidget *mScoreDisplayWidget;
    int mTatami;
    std::optional<std::pair<CategoryId, MatchId>> mCurrentMatch;
    std::optional<std::pair<CategoryId, MatchId>> mNextMatch;
    std::stack<QMetaObject::Connection> mConnections;
};

