#pragma once

#include <stack>

#include <QActionGroup>
#include <QMainWindow>
#include <QMetaObject>
#include <QPushButton>

#include "core.hpp"
#include "store_managers/client_store_manager.hpp"
#include "stores/tatami_store.hpp"
#include "widgets/match_card_widget.hpp"
#include "widgets/score_display_widget.hpp"
#include "widgets/models/actions_model.hpp"

class ScoreOperatorWindow : public QMainWindow {
    Q_OBJECT

public:
    ScoreOperatorWindow();
    void silentConnect(QString host, int port=8000); // TODO: Setup default port in config

private:
    static const int PAUSING_TIMER_INTERVAL = 100;

    void quit();
    void openHomePage();
    void openManual();
    void openReportIssue();
    void showAboutDialog();

    void clearTatamiMenu();
    void populateTatamiMenu();

    void beginResetTournament();
    void endResetTournament();

    void pausingTimerHit();

    void changeTatamis(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds);
    void beginResetMatches(CategoryId categoryId);
    void endResetMatches(CategoryId categoryId);

    void setTatami(int tatami);

    void findNextMatch();
    void updateNextButton();
    void goNextMatch();
    void disableControlButtons();
    void updateControlButtons();

    void resumeButtonClick();
    void awardIppon(MatchStore::PlayerIndex playerIndex);
    void awardWazari(MatchStore::PlayerIndex playerIndex);
    void awardShido(MatchStore::PlayerIndex playerIndex);
    void awardHansokuMake(MatchStore::PlayerIndex playerIndex);

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
    ActionsProxyModel *mActionsModel;
    int mTatami;
    std::optional<std::pair<CategoryId, MatchId>> mCurrentMatch;
    std::optional<std::pair<CategoryId, MatchId>> mNextMatch;
    std::stack<QMetaObject::Connection> mConnections;

    QPushButton *mNextButton;
    QPushButton *mResumeButton;

    QPushButton *mWhiteIpponButton;
    QPushButton *mWhiteWazariButton;
    QPushButton *mWhiteShidoButton;
    QPushButton *mWhiteHansokuMakeButton;

    QPushButton *mBlueIpponButton;
    QPushButton *mBlueWazariButton;
    QPushButton *mBlueShidoButton;
    QPushButton *mBlueHansokuMakeButton;

    QTimer mPausingTimer;
};

